#include <atomic>

#include "QCameraCaptureStream.h"

QCameraCaptureStream::QCameraCaptureStream() {
    mCamera.setViewfinder(&mSurface);

    auto cb = [&](const QVideoFrame &frame) {
        std::lock_guard<std::mutex> lock(mFrameLock);
        mFrame = frame;
    };

    mSurface.setCalback(cb);

    mBuffer = {};
}

QCameraCaptureStream::~QCameraCaptureStream() {
    mCamera.stop();
    if (mBuffer.start != nullptr) {
        std::free(mBuffer.start);
    }
}

bool QCameraCaptureStream::grab() {
    if (mCamera.state() != QCamera::ActiveState) {
        mCamera.start();
    }

    return mCamera.state() == QCamera::ActiveState;
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
    return true;
}

bool QCameraCaptureStream::open(const std::string &path) {
    // mCamera.setMedia(QUrl::fromLocalFile(QString::fromStdString(path)));
    QCameraViewfinderSettings settings = mCamera.viewfinderSettings();

    settings.setResolution(mSurface.getResolution());
    settings.setPixelFormat(mSurface.surfaceFormat().pixelFormat());
    mCamera.setViewfinderSettings(settings);

    return open();
}

uint32_t QCameraCaptureStream::getFourcc() {
    return mSurface.fourcc();
}

QSize QCameraCaptureStream::getResolution() {
    return mFrame.size();
}

bool QCameraCaptureStream::setResolution(const QSize &res) {
    QCameraViewfinderSettings settings = mCamera.viewfinderSettings();

    settings.setResolution(res);
    mCamera.setViewfinderSettings(settings);

    mSurface.setResolution(res);
    return mSurface.getResolution() == res;
}
