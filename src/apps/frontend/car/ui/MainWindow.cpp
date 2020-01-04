#include <QPainter>
#include <sys/socket.h>
#include <unistd.h>

#include <QVideoCaptureStream/QVideoCaptureStream.hpp>
#include <seraphim/image.hpp>
#include <seraphim/iop.hpp>
#include <seraphim/ipc/transport_factory.hpp>

#include <LaneDetector.pb.h>
#include <Seraphim.pb.h>

#include "MainWindow.hpp"

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
    if (!captureStream->setResolution(QSize(1280, 720))) {
        std::cout << "Failed to set capture res to 1280x720" << std::endl;
    }
    if (!captureStream->open("/home/chris/Downloads/project_video.mp4")) {
        std::cout << "Failed to open video: "
                  << "project_video.mp4" << std::endl;
    }

    mCaptureStream = std::unique_ptr<ICaptureStream>(captureStream);
    if (!mCaptureStream->start()) {
        std::cout << "Failed to start async capture stream" << std::endl;
    }

    QObject::connect(captureStream, &QVideoCaptureStream::bufferAvailable, this,
                     &MainWindow::updateBuffer);

    mLaneDetection = false;
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

void MainWindow::laneDetectionButtonClicked() {
    mLaneDetection = !mLaneDetection;
    emit toggleOverlay(mLaneDetection);
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

    if (mLaneDetection) {
        Seraphim::Message msg;
        Seraphim::Car::LaneDetector::DetectionRequest req;
        req.set_allocated_image(&img);

        // tune the ROI according to your input video
        // in this case, use a 4-point polygon shape to match the "project_video.mp4"
        // clip of the udacity course at https://github.com/udacity/CarND-Vehicle-Detection
        Seraphim::Types::Point2D *bl = req.mutable_polyroi()->add_points();
        Seraphim::Types::Point2D *tl = req.mutable_polyroi()->add_points();
        Seraphim::Types::Point2D *tr = req.mutable_polyroi()->add_points();
        Seraphim::Types::Point2D *br = req.mutable_polyroi()->add_points();
        // bottom left
        bl->set_x(210);
        bl->set_y(static_cast<int>(img.height()));
        // top left
        tl->set_x(550);
        tl->set_y(450);
        // top right
        tr->set_x(717);
        tr->set_y(450);
        // bottom right
        br->set_x(static_cast<int>(img.width()) - 210);
        br->set_y(static_cast<int>(img.height()));

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

        Seraphim::Car::LaneDetector::DetectionResponse res;
        if (!msg.res().inner().UnpackTo(&res)) {
            std::cout << "[ERROR] Failed to deserialize" << std::endl;
            return;
        }
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
