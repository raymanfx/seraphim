#include <atomic>

#include "QVideoCaptureStream.h"

QVideoCaptureStream::QVideoCaptureStream() {
    mPlayer.setVideoOutput(&mSurface);
    mBuffer = {};

    QObject::connect(&mSurface, &QVideoCaptureSurface::frameAvailable, this,
                     &QVideoCaptureStream::consumeFrame);
}

QVideoCaptureStream::~QVideoCaptureStream() {
    mPlayer.stop();
    if (mBuffer.start != nullptr) {
        std::free(mBuffer.start);
    }
}

bool QVideoCaptureStream::grab() {
    if (mPlayer.state() != QMediaPlayer::PlayingState) {
        mPlayer.play();
    }

    return mPlayer.state() == QMediaPlayer::PlayingState;
}

bool QVideoCaptureStream::retrieve(struct Buffer &buf) {
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

void QVideoCaptureStream::consumeFrame(const QVideoFrame &frame) {
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

    emit bufferAvailable(buf);
    mFrame.unmap();
}

bool QVideoCaptureStream::open(const std::string &path) {
    mPlayer.setMedia(QUrl::fromLocalFile(QString::fromStdString(path)));

    return open();
}

uint32_t QVideoCaptureStream::getFourcc() {
    // https://chromium.googlesource.com/libyuv/libyuv/+/refs/heads/master/include/libyuv/video_common.h
    QVideoFrame::PixelFormat format = mSurface.surfaceFormat().pixelFormat();
    switch (format) {
    case QVideoFrame::Format_RGB24:
        return fourcc('R', 'G', 'B', '3');
    default:
        return 0;
    }
}

QSize QVideoCaptureStream::getResolution() {
    return mFrame.size();
}

bool QVideoCaptureStream::setResolution(const QSize &res) {
    mSurface.setResolution(res);

    return mSurface.getResolution() == res;
}
