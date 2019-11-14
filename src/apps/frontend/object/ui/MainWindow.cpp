#include <QPainter>
#include <sys/socket.h>
#include <unistd.h>

#include <ImageUtilsQt.h>
#include <QCameraCaptureStream/QCameraCaptureStream.h>
#include <seraphim/image.h>
#include <seraphim/ipc/transport_factory.h>

#include <ObjectDetector.pb.h>
#include <Seraphim.pb.h>

#include "MainWindow.h"
#include "coco.h"

/**
 * @brief Get the class label for a given id.
 * @param classid Class id from a net forward pass.
 * @return Label as string or empty string if the classid does not exist.
 */
static std::string label_coco(const int &classid) {
    return sph::object::data::MSCOCO_2014.find(classid) == sph::object::data::MSCOCO_2014.end()
               ? ""
               : sph::object::data::MSCOCO_2014.at(classid);
}

static void draw_predictions(QImage &overlay,
                             const Seraphim::Object::Detector::DetectionResponse &res) {
    QPainter painter(&overlay);
    QColor color;
    std::string label;

    QFont font = painter.font();
    font.setPointSize(20);
    painter.setFont(font);

    for (int i = 0; i < res.labels().size(); i++) {
        if (res.labels(i) <= 30) {
            color = Qt::red;
        } else if (res.labels(i) <= 60) {
            color = Qt::green;
        } else if (res.labels(i) <= 90) {
            color = Qt::blue;
        }

        painter.setPen(color);
        painter.drawRect(res.rois(i).x(), res.rois(i).y(), res.rois(i).w(), res.rois(i).h());
        label = std::to_string(static_cast<int>(res.confidences(i) * 100)) + "%";
        if (label_coco(res.labels(i)) != "") {
            label = label_coco(res.labels(i)) + ": " + label;
        }

        painter.drawText(res.rois(i).x(), res.rois(i).y(), QString::fromStdString(label));
    }
}

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

    mObjectRecognition = false;
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

    if (mObjectRecognition) {
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
    } else if (!mObjectRecognition) {
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
    sph::Pixelformat::Enum pixfmt;

    pixfmt = sph::Pixelformat::uid(buf.format.fourcc);
    if (pixfmt == sph::Pixelformat::Enum::UNKNOWN) {
        return;
    }

    img = sph::CoreImage(static_cast<unsigned char *>(buf.start), buf.format.width,
                         buf.format.height, pixfmt, buf.format.stride);
    if (!img) {
        return;
    }

    if (!sph::frontend::Image2QImage(img, mFrame)) {
        return;
    }
}

void MainWindow::objectRecognitionButtonClicked() {
    mObjectRecognition = !mObjectRecognition;
    emit toggleOverlay(mObjectRecognition);
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

    if (mObjectRecognition) {
        Seraphim::Message msg;
        Seraphim::Object::Detector::DetectionRequest req;
        req.set_allocated_image(&img);

        // force at least 0.5 confidence
        req.set_confidence(0.5f);

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

        Seraphim::Object::Detector::DetectionResponse res;
        if (!msg.res().inner().UnpackTo(&res)) {
            std::cout << "[ERROR] Failed to deserialize" << std::endl;
            return;
        }
        std::cout << "Server sent response:" << std::endl
                  << "  status=" << msg.res().status() << std::endl
                  << "  objects=" << res.labels().size() << std::endl;

        // clear overlay
        overlay.fill(Qt::transparent);

        // draw the new overlay
        draw_predictions(overlay, res);
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
