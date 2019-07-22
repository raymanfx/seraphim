#include <QPainter>
#include <sys/socket.h>
#include <unistd.h>

#include <Seraphim.pb.h>
#include <V4L2CaptureStream/V4L2CaptureStream.h>
#include <seraphim/ipc/transport_factory.h>

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
                             const Seraphim::Object::Classifier::ClassificationResponse &res) {
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

    mObjectRecognition = false;
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

    mTransport->set_timeout(1000);
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

    if (mObjectRecognition) {
        Seraphim::Message msg;
        Seraphim::Object::Classifier::ClassificationRequest *req =
            msg.mutable_req()->mutable_object()->mutable_classifier()->mutable_classification();
        req->set_allocated_image(img);

        // force at least 0.5 confidence
        req->set_confidence(0.5f);

        Seraphim::Object::Classifier::ClassificationResponse res;

        if (!mTransport->send(msg)) {
            std::cout << "Transport: failed to send(): " << strerror(errno) << std::endl;
        }
        mTransport->recv(msg);
        res = msg.res().object().classifier().classification();

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
