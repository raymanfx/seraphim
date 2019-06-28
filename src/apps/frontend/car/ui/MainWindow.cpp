#include <QPainter>
#include <sys/socket.h>
#include <unistd.h>

#include <QVideoCaptureStream/QVideoCaptureStream.h>
#include <Seraphim.pb.h>

#include "MainWindow.h"

static void draw_lanes(QImage &overlay, const Seraphim::Car::LaneDetector::DetectionResponse &res) {
    QPainter painter(&overlay);
    QColor color(255, 0, 0, 128);
    QBrush brush = painter.brush();

    painter.setPen(color);
    brush.setColor(color);
    brush.setStyle(Qt::SolidPattern);
    painter.setBrush(brush);

    for (const auto &lane : res.lanes()) {
        QVector<QPoint> polyPoints = { QPoint(lane.bottomleft().x(), lane.bottomleft().y()),
                                       QPoint(lane.topleft().x(), lane.topleft().y()),
                                       QPoint(lane.topright().x(), lane.topright().y()),
                                       QPoint(lane.bottomright().x(), lane.bottomright().y()) };
        painter.drawLine(lane.bottomleft().x(), lane.bottomleft().y(), lane.topleft().x(),
                         lane.topleft().y());
        painter.drawLine(lane.bottomright().x(), lane.bottomright().y(), lane.topright().x(),
                         lane.topright().y());
        painter.drawPolygon(polyPoints);
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

    QVideoCaptureStream *captureStream = new QVideoCaptureStream();
    if (!captureStream->setWidth(1280)) {
        std::cout << "Failed to set capture width to 1280" << std::endl;
    }
    if (!captureStream->setHeight(720)) {
        std::cout << "Failed to set capture height to 1280" << std::endl;
    }
    if (!captureStream->open("project_video.mp4")) {
        std::cout << "Failed to open video: "
                  << "project_video.mp4" << std::endl;
    }
    mCaptureStream = std::unique_ptr<ICaptureStream>(captureStream);

    mLaneDetection = false;
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

    if (mLaneDetection) {
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
    } else if (!mLaneDetection) {
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

void MainWindow::laneDetectionButtonClicked() {
    mLaneDetection = !mLaneDetection;
    emit toggleOverlay(mLaneDetection);
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

    if (mLaneDetection) {
        Seraphim::Message msg;
        Seraphim::Car::LaneDetector::DetectionRequest *req =
            msg.mutable_req()->mutable_car()->mutable_detector()->mutable_detection();
        req->set_allocated_image(img);

        Seraphim::Car::LaneDetector::DetectionResponse res;

        if (!mTransport->send(msg)) {
            std::cout << "Transport: failed to send(): " << strerror(errno) << std::endl;
        }
        mTransport->recv(msg);
        res = msg.res().car().detector().detection();

        std::cout << "Server sent response:" << std::endl
                  << "  status=" << msg.res().status() << std::endl
                  << "  lanes=" << res.lanes().size() << std::endl;

        // clear overlay
        overlay.fill(Qt::transparent);

        // draw the new overlay
        draw_lanes(overlay, res);
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
