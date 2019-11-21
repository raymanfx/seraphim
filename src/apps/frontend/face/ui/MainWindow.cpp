#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

#include <QCameraCaptureStream/QCameraCaptureStream.h>
#include <QPainter>
#include <seraphim/image.h>
#include <seraphim/iop.h>
#include <seraphim/ipc/transport_factory.h>

#include <FaceDetector.pb.h>
#include <FaceRecognizer.pb.h>
#include <FacemarkDetector.pb.h>
#include <Seraphim.pb.h>

#include "MainWindow.h"

MainWindow::MainWindow(QObject *parent) : QObject(parent) {
    // register image provider
    mEngine = new QQmlApplicationEngine(this);
    mMainImageProvider = new QImageProvider(this);
    mOverlayImageProvider = new QImageProvider(this);

    mEngine->rootContext()->setContextProperty("mainViewerObj", mMainImageProvider);
    mEngine->rootContext()->setContextProperty("overlayViewerObj", mOverlayImageProvider);
    mEngine->rootContext()->setContextProperty("mainWindow", this);

    mEngine->addImageProvider(QLatin1String("mainViewer"), mMainImageProvider);
    mEngine->addImageProvider(QLatin1String("overlayViewer"), mOverlayImageProvider);

    // kick off timer for 33ms (30 FPS)
    mMainTimer = new QTimer(this);
    connect(mMainTimer, SIGNAL(timeout()), SLOT(updateTimeout()));
    mMainTimer->start(33);
    mMainLoopDuration = 0;

    // debug output timer
    mDiagPrintTimer = new QTimer(this);
    connect(mDiagPrintTimer, SIGNAL(timeout()), SLOT(diagTimeout()));
    mDiagPrintTimer->start(500);

    QCameraCaptureStream *captureStream = new QCameraCaptureStream();
    if (captureStream->open()) {
        if (!captureStream->setFourcc(QCameraCaptureStream::fourcc('M', 'J', 'P', 'G'))) {
            std::cout << "Failed to set capture format to MJPG" << std::endl;
        }
        if (!captureStream->setResolution(QSize(1280, 720))) {
            std::cout << "Failed to set capture res to 1280x720" << std::endl;
        }
    } else {
        std::cout << "Failed to open default capture device (0)" << std::endl;
    }

    mCaptureStream = std::unique_ptr<ICaptureStream>(captureStream);
    if (!mCaptureStream->start()) {
        std::cout << "Failed to start async capture stream" << std::endl;
    }

    QObject::connect(captureStream, &QCameraCaptureStream::bufferAvailable, this,
                     &MainWindow::updateBuffer);

    mFaceStorage = new FaceStorage();
    mEngine->addImageProvider(QLatin1String("faceStorage"), mFaceStorage);

    mFaceTraining = 0;
    mFaceTrainingSamples = 10;
    mFaceTrained = false;
    mFaceLabel = 0;
    mFaceDetection = false;
    mFacemarkDetection = false;
    mFaceRecognition = false;
    mBackendWorkerActive = false;
    mBackendFrameReady = false;
    mBackendSync = false;
}

MainWindow::~MainWindow() {
    mCaptureStream->stop();
    mFrame = QImage();

    if (mBackendWorkerActive) {
        mBackendWorkerActive = false;
        if (mBackendWorker.joinable()) {
            mBackendWorker.join();
        }
    }
}

bool MainWindow::load() {
    mEngine->load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (mEngine->rootObjects().isEmpty()) {
        return false;
    }

    // setup initial empty overlay
    mOverlay = QImage(1, 1, QImage::Format_ARGB32);
    mOverlay.fill(Qt::transparent);
    mOverlayImageProvider->show(mOverlay);

    return true;
}

void MainWindow::diagTimeout() {
    QString text;
    text += "Main: " + QString::number(mMainLoopDuration) + "ms";
    text += mDiagBuffer;
    mDiagBuffer.clear();

    emit printDiag(text);
}

void MainWindow::updateTimeout() {
    std::chrono::high_resolution_clock::time_point start =
        std::chrono::high_resolution_clock::now();

    // TODO: frame buffer?
    if (mFrame.isNull()) {
        return;
    }

    if (mFaceDetection || mFacemarkDetection || mFaceRecognition || mFaceTraining) {
        if (!mBackendWorkerActive) {
            mBackendWorkerActive = true;
            mBackendWorker = std::thread([&]() {
                while (mBackendWorkerActive) {
                    backendWork();
                }
            });
        }

        if (mBackendSync) {
            std::unique_lock<std::mutex> lock(mBackendLock);
            mBackendNotifier.wait(lock, [&] { return mBackendFrameReady; });
            mBackendFrameReady = false;
        }

        {
            std::lock_guard<std::mutex> lock(mOverlayLock);
            if (!mOverlay.isNull()) {
                mOverlayImageProvider->show(mOverlay);
            }
        }
    } else if (!mFaceDetection && !mFacemarkDetection && !mFaceRecognition && !mFaceTraining) {
        if (mBackendWorkerActive) {
            mBackendWorkerActive = false;
            if (mBackendWorker.joinable()) {
                mBackendWorker.join();
            }
        }
    }

    mMainImageProvider->show(mFrame);

    mMainLoopDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::high_resolution_clock::now() - start)
                            .count();
}

void MainWindow::updateBuffer(const ICaptureStream::Buffer &buf) {
    std::lock_guard<std::mutex> lock(mFrameLock);
    mCaptureBuffer = buf;

    // get the QImage wrapper representation
    sph::CoreImage img;
    sph::Pixelformat pixfmt;

    pixfmt = sph::Pixelformat(buf.format.fourcc);
    if (!pixfmt) {
        return;
    }

    img = sph::CoreImage(static_cast<unsigned char *>(buf.start), buf.format.width,
                         buf.format.height, pixfmt, buf.format.stride);

    mFrame = sph::iop::qt::from_image(img);
}

void MainWindow::faceDetectionButtonClicked() {
    mFaceDetection = !mFaceDetection;
    emit toggleOverlay(mFaceDetection || mFacemarkDetection);
}

void MainWindow::facemarkDetectionButtonClicked() {
    mFacemarkDetection = !mFacemarkDetection;
    emit toggleOverlay(mFacemarkDetection || mFaceDetection);
}

void MainWindow::faceButtonClicked(int label) {
    mFaceLabel = label;
    mFaceTraining = mFaceTrainingSamples;
}

void MainWindow::faceRecognitionButtonClicked() {
    mFaceRecognition = !mFaceRecognition;
}

void MainWindow::toggleBackendSync(bool enable) {
    mBackendSync = enable;
}

bool MainWindow::openTransportSession(QString uri) {
    mTransport = sph::ipc::TransportFactory::Instance().open(uri.toStdString());
    if (mTransport == nullptr) {
        return false;
    }

    mTransport->set_rx_timeout(1000);
    mTransport->set_tx_timeout(1000);
    return true;
}

void MainWindow::backendWork() {
    Seraphim::Types::Image2D img;
    std::vector<unsigned char> framebuffer;
    QImage overlay(mFrame.size(), QImage::Format_ARGB32);

    {
        std::lock_guard<std::mutex> lock(mFrameLock);
        if (mFrame.isNull()) {
            std::lock_guard<std::mutex> lock2(mOverlayLock);
            mOverlay.fill(Qt::transparent);
            return;
        }

        // copy the current frame so we can send its data to the backend
        framebuffer.resize(mCaptureBuffer.size);
        std::memcpy(&framebuffer[0], mCaptureBuffer.start, mCaptureBuffer.size);
        img.set_data(reinterpret_cast<char *>(&framebuffer[0]), framebuffer.size());
        img.set_fourcc(mCaptureBuffer.format.fourcc);
        img.set_width(mCaptureBuffer.format.width);
        img.set_height(mCaptureBuffer.format.height);
        img.set_stride(mCaptureBuffer.format.stride);
    }

    if (mFaceDetection) {
        Seraphim::Message msg;
        Seraphim::Face::FaceDetector::DetectionRequest req;
        req.set_allocated_image(&img);

        msg.mutable_req()->mutable_inner()->PackFrom(req);
        try {
            mTransport->send(msg);
            // we still need the image, keep protobuf from deleting it by releasing it manually
            req.release_image();
            mTransport->receive(msg);
        } catch (std::exception &e) {
            std::cout << "[ERROR] Transport I/O error: " << e.what() << std::endl;
            return;
        }

        Seraphim::Face::FaceDetector::DetectionResponse res;
        if (!msg.res().inner().UnpackTo(&res)) {
            std::cout << "[ERROR] Failed to deserialize" << std::endl;
            return;
        }
        std::cout << "Server sent response:" << std::endl
                  << "  status=" << msg.res().status() << std::endl
                  << "  faces=" << res.faces_size() << std::endl;

        // clear overlay
        overlay.fill(Qt::transparent);

        // draw the new overlay
        QPainter painter(&overlay);
        painter.setPen(Qt::red);
        for (const auto &face : res.faces()) {
            painter.drawRect(face.x(), face.y(), face.w(), face.h());
        }
    }

    if (mFacemarkDetection) {
        Seraphim::Message msg;
        Seraphim::Face::FacemarkDetector::DetectionRequest req;
        req.set_allocated_image(&img);

        msg.mutable_req()->mutable_inner()->PackFrom(req);
        try {
            mTransport->send(msg);
            // we still need the image, keep protobuf from deleting it by releasing it manually
            req.release_image();
            mTransport->receive(msg);
        } catch (std::exception &e) {
            std::cout << "[ERROR] Transport I/O error: " << e.what() << std::endl;
            return;
        }

        Seraphim::Face::FacemarkDetector::DetectionResponse res;
        if (!msg.res().inner().UnpackTo(&res)) {
            std::cout << "[ERROR] Failed to deserialize" << std::endl;
            return;
        }
        std::cout << "Server sent response:" << std::endl
                  << "  status=" << msg.res().status() << std::endl
                  << "  faces=" << res.faces_size() << std::endl;

        // clear overlay
        overlay.fill(Qt::transparent);

        // draw the new overlay
        QPainter painter(&overlay);
        painter.setBrush(QBrush(Qt::red));
        for (const auto &facemarks : res.facemarks()) {
            for (const auto &pointset : facemarks.pointsets()) {
                for (const auto &point : pointset.points()) {
                    painter.drawEllipse(point.x(), point.y(), 10, 10);
                }
            }
        }
    }

    if (mFaceRecognition) {
        QString diagInfo = "";

        int label;
        double confidence;

        Seraphim::Message msg;
        Seraphim::Face::FaceRecognizer::PredictionRequest req;
        req.set_allocated_image(&img);

        msg.mutable_req()->mutable_inner()->PackFrom(req);
        try {
            mTransport->send(msg);
            // we still need the image, keep protobuf from deleting it by releasing it manually
            req.release_image();
            mTransport->receive(msg);
        } catch (std::exception &e) {
            std::cout << "[ERROR] Transport I/O error: " << e.what() << std::endl;
            return;
        }

        Seraphim::Face::FaceRecognizer::PredictionResponse res;
        if (!msg.res().inner().UnpackTo(&res)) {
            std::cout << "[ERROR] Failed to deserialize" << std::endl;
            return;
        }
        std::cout << "Server sent response:" << std::endl
                  << "  status=" << msg.res().status() << std::endl
                  << "  faces=" << res.labels_size() << std::endl;

        for (int i = 0; i < res.labels_size(); i++) {
            std::cout << "  label=" << res.labels(0) << std::endl
                      << "  distance=" << res.distances(0) << std::endl;

            label = res.labels(i);
            confidence = res.distances(i);
            if (label == -1) {
                std::cout << "Face not recognized.." << std::endl;
            }

            diagInfo += "\n";
            diagInfo += "Label ";
            diagInfo += QString::number(label);
            diagInfo += ": distance=";
            diagInfo += QString::number(confidence);
            emit recognized(label, confidence);
        }

        if (mDiagBuffer.isEmpty()) {
            mDiagBuffer += diagInfo;
        }
    }

    if (mFaceTraining > 0) {
        Seraphim::Message msg;
        Seraphim::Face::FaceRecognizer::TrainingRequest req;
        req.set_label(mFaceLabel);
        req.set_allocated_image(&img);
        req.set_invalidate(mFaceTraining == 10);

        msg.mutable_req()->mutable_inner()->PackFrom(req);
        try {
            mTransport->send(msg);
            // we still need the image, keep protobuf from deleting it by releasing it manually
            req.release_image();
            mTransport->receive(msg);
        } catch (std::exception &e) {
            std::cout << "[ERROR] Transport I/O error: " << e.what() << std::endl;
            return;
        }

        Seraphim::Face::FaceRecognizer::TrainingResponse res;
        if (!msg.res().inner().UnpackTo(&res)) {
            std::cout << "[ERROR] Failed to deserialize" << std::endl;
            return;
        }
        std::cout << "Server sent response:" << std::endl
                  << "  status=" << msg.res().status() << std::endl
                  << "  label=" << res.label() << std::endl;

        if (res.label() >= 0) {
            Seraphim::Types::Region2D region = res.face();
            QRect roi(region.x(), region.y(), region.w(), region.h());
            mFaceLabels.push_back(mFaceLabel);

            if (mFaceTraining == 10) {
                // TODO: just grayscale for now
                QImage face = mFrame.copy(roi);
                mFaceStorage->setFace(mFaceLabels[0], face);
            }

            mFaceTraining--;
            emit training(mFaceLabels[0],
                          1.0f * (mFaceTrainingSamples - mFaceTraining) / mFaceTrainingSamples);

            if (mFaceTraining == 0) {
                std::cout << "Training complete, label: " << res.label() << std::endl;
                mFaceTrained = true;
                mFaceLabels.clear();
            }
        }
    }

    // swap the new overlay into the class buffer
    {
        std::lock_guard<std::mutex> lock(mOverlayLock);
        if (overlay.isNull()) {
            overlay.fill(Qt::transparent);
        }
        overlay.swap(mOverlay);
    }

    std::lock_guard<std::mutex> lock(mBackendLock);
    mBackendFrameReady = true;
    mBackendNotifier.notify_one();
}
