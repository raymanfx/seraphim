#ifndef QVIDEO_CAPTURE_STREAM_H
#define QVIDEO_CAPTURE_STREAM_H

#include <ICaptureStream/ICaptureStream.h>
#include <QtMultimedia/QAbstractVideoSurface>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QVideoSurfaceFormat>
#include <cstdint>
#include <cstring>
#include <mutex>

class QVideoCaptureSurface : public QAbstractVideoSurface {
public:
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
        QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const {
        Q_UNUSED(handleType);

        // Return the formats you will support
        return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_RGB24;
    }

    bool present(const QVideoFrame &frame) {
        if (mCallback) {
            mCallback(frame);
        }

        return true;
    }

    QSize getResolution() const { return nativeResolution(); }
    void setResolution(const QSize &resolution) { setNativeResolution(resolution); }
    void setCalback(std::function<void(const QVideoFrame &frame)> cb) { mCallback = cb; }

private:
    std::function<void(const QVideoFrame &frame)> mCallback;
};

class QVideoCaptureStream : public ICaptureStream {
public:
    QVideoCaptureStream();
    virtual ~QVideoCaptureStream() override;

    bool isOpen() override { return mPlayer.mediaStatus() == QMediaPlayer::LoadedMedia; }

    bool open() override { return mPlayer.mediaStatus() == QMediaPlayer::LoadedMedia; }
    bool close() override {
        mSurface.stop();
        return true;
    }

    bool grab() override;
    bool retrieve(struct Buffer &buf) override;

    // convenience API
    bool open(const std::string &path);

    // convenience API
    uint32_t getFourcc();
    uint32_t getWidth();
    uint32_t getHeight();
    bool setFourcc(const uint32_t &fourcc) {
        (void)fourcc;
        return false;
    }
    bool setWidth(const uint32_t &width);
    bool setHeight(const uint32_t &height);

    // helpers
    static inline uint32_t fourcc(const char &a, const char &b, const char &c, const char &d) {
        return ((static_cast<uint32_t>(a) << 0) | (static_cast<uint32_t>(b) << 8) |
                (static_cast<uint32_t>(c) << 16) | (static_cast<uint32_t>(d) << 24));
    }

private:
    /// internal surface for Qt abstraction
    QVideoCaptureSurface mSurface;

    /// media player engine
    QMediaPlayer mPlayer;

    /// surface frame buffer
    QVideoFrame mFrame;
    std::mutex mFrameLock;

    /// copy buffer
    ICaptureStream::Buffer mBuffer;
};

#endif // QVIDEO_CAPTURE_STREAM_H
