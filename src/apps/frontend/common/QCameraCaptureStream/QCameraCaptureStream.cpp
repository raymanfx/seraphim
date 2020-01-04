#include <QCameraInfo>
#include <atomic>

#include "QCameraCaptureStream.hpp"

QCameraCaptureStream::QCameraCaptureStream() {
    mCamera = nullptr;
    mBuffer = {};

    QObject::connect(&mSurface, &QCameraCaptureSurface::frameAvailable, this,
                     &QCameraCaptureStream::consumeFrame);
}

QCameraCaptureStream::~QCameraCaptureStream() {
    mCamera->stop();

    if (mBuffer.start != nullptr) {
        std::free(mBuffer.start);
    }
}

bool QCameraCaptureStream::open() {
    QCameraInfo defaultCamera;
    QCameraViewfinderSettings settings;
    QCameraFocus *focus;

    if (!mCamera) {
        defaultCamera = QCameraInfo::defaultCamera();
        if (defaultCamera.isNull()) {
            return false;
        } else {
            mCamera = new QCamera(defaultCamera);
        }
    }

    settings = mCamera->viewfinderSettings();
    settings.setResolution(mSurface.getResolution());
    settings.setPixelFormat(mSurface.surfaceFormat().pixelFormat());

    mCamera->setViewfinder(&mSurface);
    mCamera->setViewfinderSettings(settings);

    // force continuous autofocus for now
    focus = mCamera->focus();
    focus->setFocusMode(QCameraFocus::ContinuousFocus);

    // unfortunately Qt does not allow us to block until the camera is ready here?
    // mCamera->load();
    // return mCamera->status() == QCamera::LoadedStatus;
    return true;
}

bool QCameraCaptureStream::close() {
    mSurface.stop();
    return true;
}

bool QCameraCaptureStream::grab() {
    if (mCamera->state() != QCamera::ActiveState) {
        mCamera->start();
    }

    return mCamera->state() == QCamera::ActiveState;
}

bool QCameraCaptureStream::retrieve(struct Buffer &buf) {
    std::lock_guard<std::mutex> lock(mFrameLock);

    if (!mFrame.isValid()) {
        return false;
    }

    if (!mFrame.map(QAbstractVideoBuffer::ReadOnly)) {
        return false;
    }

    // reallocate buffer if necessary
    if (mBuffer.size < static_cast<size_t>(mFrame.mappedBytes())) {
        if (mBuffer.start != nullptr) {
            std::free(mBuffer.start);
        }
        mBuffer.start = std::malloc(static_cast<size_t>(mFrame.mappedBytes()));
        mBuffer.size = static_cast<size_t>(mFrame.mappedBytes());
    }

    mBuffer.bytesused = static_cast<size_t>(mFrame.mappedBytes());
    std::memcpy(mBuffer.start, mFrame.bits(), mBuffer.bytesused);
    mFrame.unmap();

    buf.start = mBuffer.start;
    buf.size = mBuffer.size;
    buf.bytesused = mBuffer.bytesused;
    buf.format.width = static_cast<uint32_t>(getResolution().width());
    buf.format.height = static_cast<uint32_t>(getResolution().height());
    buf.format.fourcc = getFourcc();
    buf.format.stride = static_cast<uint32_t>(mFrame.bytesPerLine());
    return true;
}

void QCameraCaptureStream::consumeFrame(const QVideoFrame &frame) {
    Buffer buf = {};
    std::lock_guard<std::mutex> lock(mFrameLock);

    mFrame = frame;
    if (!mFrame.map(QAbstractVideoBuffer::ReadOnly)) {
        return;
    }

    buf.start = mFrame.bits();
    buf.size = static_cast<size_t>(mFrame.mappedBytes());
    buf.bytesused = buf.size;
    buf.format.width = static_cast<uint32_t>(getResolution().width());
    buf.format.height = static_cast<uint32_t>(getResolution().height());
    buf.format.fourcc = getFourcc();
    buf.format.stride = static_cast<uint32_t>(mFrame.bytesPerLine());

    emit bufferAvailable(buf);
    mFrame.unmap();
}

bool QCameraCaptureStream::open(const std::string &deviceName) {
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QCameraInfo camera;

    for (const auto &cam : cameras) {
        if (cam.deviceName() == QString::fromStdString(deviceName)) {
            camera = cam;
        }
    }

    if (camera.isNull()) {
        return false;
    }

    if (mCamera) {
        delete mCamera;
    }

    mCamera = new QCamera(camera);
    return open();
}

uint32_t QCameraCaptureStream::getFourcc() {
    // https://chromium.googlesource.com/libyuv/libyuv/+/refs/heads/master/include/libyuv/video_common.h
    QVideoFrame::PixelFormat format = mSurface.surfaceFormat().pixelFormat();
    switch (format) {
    case QVideoFrame::Format_RGB24:
        return fourcc('R', 'G', 'B', '3');
    case QVideoFrame::Format_Jpeg:
        return fourcc('M', 'J', 'P', 'G');
    default:
        return 0;
    }
}

QSize QCameraCaptureStream::getResolution() {
    return mFrame.size();
}

bool QCameraCaptureStream::setResolution(const QSize &res) {
    QCameraViewfinderSettings settings = mCamera->viewfinderSettings();

    settings.setResolution(res);
    mCamera->setViewfinderSettings(settings);

    mSurface.setResolution(res);
    return mSurface.getResolution() == res;
}
