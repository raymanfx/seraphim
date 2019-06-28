#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

#include <FaceDetector.pb.h>
#include <FaceRecognizer.pb.h>
#include <QPainter>
#include <Seraphim.pb.h>
#include <V4L2CaptureStream/V4L2CaptureStream.h>

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

    V4L2CaptureStream *captureStream = new V4L2CaptureStream();
    if (captureStream->open(0)) {
        if (!captureStream->setFourcc(V4L2CaptureStream::fourcc_from_string('M', 'J', 'P', 'G'))) {
            std::cout << "Failed to set capture format to MJPG" << std::endl;
        }
        if (!captureStream->setWidth(1280)) {
            std::cout << "Failed to set capture width to 1280" << std::endl;
        }
        if (!captureStream->setHeight(720)) {
            std::cout << "Failed to set capture height to 720" << std::endl;
        }
    } else {
        std::cout << "Failed to open default capture device (0)" << std::endl;
    }
    mCaptureStream = std::unique_ptr<ICaptureStream>(captureStream);

    mFaceStorage = new FaceStorage();
    mEngine->addImageProvider(QLatin1String("faceStorage"), mFaceStorage);

    mFaceTraining = 0;
    mFaceTrainingSamples = 10;
    mFaceTrained = false;
    mFaceLabel = 0;
    mFaceDetection = false;
    mFaceRecognition = false;
    mBackendWorkerActive = false;
    mBackendFrameReady = false;
    mBackendSync = false;
}

MainWindow::~MainWindow() {
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
    struct ICaptureStream::Buffer buf = {};
    std::chrono::high_resolution_clock::time_point start =
        std::chrono::high_resolution_clock::now();

    {
        std::lock_guard<std::mutex> lock(mFrameLock);
        if (!mCaptureStream->read(buf)) {
            std::cout << "Failed to capture frame from stream" << std::endl;
            mFrame = QImage();
            return;
        }

        // save the buffer for the backend worker thread
        mCaptureBuffer = buf;

        // get the QImage wrapper representation
        mFrame = QImageProvider::QImageFromBuffer(reinterpret_cast<uchar *>(buf.start),
                                                  buf.bytesused, buf.format.width,
                                                  buf.format.height, buf.format.fourcc);
    }

    if (mFrame.isNull()) {
        return;
    }

    if (mFaceDetection || mFaceRecognition || mFaceTraining) {
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
    } else if (!mFaceDetection && !mFaceRecognition && !mFaceTraining) {
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

void MainWindow::faceButtonClicked(int label) {
    mFaceLabel = label;
    mFaceTraining = mFaceTrainingSamples;
}

void MainWindow::faceDetectionButtonClicked() {
    mFaceDetection = !mFaceDetection;
    emit toggleOverlay(mFaceDetection);
}

void MainWindow::faceRecognitionButtonClicked() {
    mFaceRecognition = !mFaceRecognition;
}

void MainWindow::toggleBackendSync(bool enable) {
    mBackendSync = enable;
}

bool MainWindow::openShmSession(QString path) {
    sph::ipc::SharedMemoryTransport *shm = new sph::ipc::SharedMemoryTransport();
    if (!shm->open(path.toStdString())) {
        std::cout << "Failed to open SHM: " << strerror(errno) << std::endl;
        return false;
    }

    shm->set_timeout(1000);
    mTransport = std::unique_ptr<sph::ipc::ITransport>(shm);
    return true;
}

bool MainWindow::openTcpSession(QString ip, ushort port) {
    sph::ipc::TCPTransport *tcp = new sph::ipc::TCPTransport(AF_INET);
    if (!tcp->connect(ip.toStdString(), port)) {
        std::cout << "Failed to connect to TCP: " << strerror(errno) << std::endl;
        return false;
    }

    mTransport = std::unique_ptr<sph::ipc::ITransport>(tcp);
    return true;
}

void MainWindow::backendWork() {
    Seraphim::Types::Image2D *img = new Seraphim::Types::Image2D;
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
        img->set_data(reinterpret_cast<char *>(&framebuffer[0]), framebuffer.size());
        img->set_fourcc(mCaptureBuffer.format.fourcc);
        img->set_width(mCaptureBuffer.format.width);
        img->set_height(mCaptureBuffer.format.height);
    }

    if (mFaceDetection) {
        Seraphim::Message msg;
        Seraphim::Face::Detector::DetectionRequest *req =
            msg.mutable_req()->mutable_face()->mutable_detector()->mutable_detection();
        req->set_allocated_image(img);

        Seraphim::Face::Detector::DetectionResponse res;

        if (!mTransport->send(msg)) {
            std::cout << "Transport: failed to send(): " << strerror(errno) << std::endl;
        }
        mTransport->recv(msg);
        res = msg.res().face().detector().detection();

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
        Seraphim::Face::Recognizer::RecognitionRequest *req =
            msg.mutable_req()->mutable_face()->mutable_recognizer()->mutable_recognition();
        req->set_allocated_image(img);

        Seraphim::Face::Recognizer::RecognitionResponse res;

        if (!mTransport->send(msg)) {
            std::cout << "Transport: failed to send(): " << strerror(errno) << std::endl;
        }
        mTransport->recv(msg);
        res = msg.res().face().recognizer().recognition();

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
        Seraphim::Face::Recognizer::TrainingRequest *req =
            msg.mutable_req()->mutable_face()->mutable_recognizer()->mutable_training();
        req->set_label(mFaceLabel);
        req->set_allocated_image(img);
        req->set_invalidate(mFaceTraining == 10);

        Seraphim::Face::Recognizer::TrainingResponse res;

        if (!mTransport->send(msg)) {
            std::cout << "Transport: failed to send(): " << strerror(errno) << std::endl;
        }
        mTransport->recv(msg);
        res = msg.res().face().recognizer().training();

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
