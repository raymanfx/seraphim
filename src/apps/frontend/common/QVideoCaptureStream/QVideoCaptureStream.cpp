#include <atomic>

#include "QVideoCaptureStream.h"

QVideoCaptureStream::QVideoCaptureStream() {
    mPlayer.setVideoOutput(&mSurface);

    auto cb = [&](const QVideoFrame &frame) {
        std::lock_guard<std::mutex> lock(mFrameLock);
        mFrame = frame;
    };

    mSurface.setCalback(cb);

    mBuffer = {};
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
    buf.format.width = getWidth();
    buf.format.height = getHeight();
    buf.format.fourcc = getFourcc();
    return true;
}

bool QVideoCaptureStream::open(const std::string &path) {
    mPlayer.setMedia(QUrl::fromLocalFile(QString::fromStdString(path)));

    return open();
}

uint32_t QVideoCaptureStream::getFourcc() {
    return mSurface.fourcc();
}

uint32_t QVideoCaptureStream::getWidth() {
    return static_cast<uint32_t>(mFrame.width());
}

uint32_t QVideoCaptureStream::getHeight() {
    return static_cast<uint32_t>(mFrame.height());
}

bool QVideoCaptureStream::setWidth(const uint32_t &width) {
    QSize res = mSurface.nativeResolution();

    res.setWidth(static_cast<int>(width));
    mSurface.setResolution(res);

    return static_cast<uint32_t>(mSurface.getResolution().width()) == width;
}

bool QVideoCaptureStream::setHeight(const uint32_t &height) {
    QSize res = mSurface.nativeResolution();

    res.setHeight(static_cast<int>(height));
    mSurface.setResolution(res);

    return static_cast<uint32_t>(mSurface.getResolution().height()) == height;
}
