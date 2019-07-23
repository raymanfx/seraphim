#ifndef QCAMERA_CAPTURE_STREAM_H
#define QCAMERA_CAPTURE_STREAM_H

#include <ICaptureStream/ICaptureStream.h>
#include <QtMultimedia/QAbstractVideoSurface>
#include <QtMultimedia/QCamera>
#include <QtMultimedia/QVideoSurfaceFormat>
#include <cstdint>
#include <cstring>
#include <mutex>

class QCameraCaptureSurface : public QAbstractVideoSurface {
public:
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
        QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const {
        Q_UNUSED(handleType);

        // Return the formats you will support
        return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_Jpeg;
    }

    bool present(const QVideoFrame &frame) {
        if (mCallback) {
            mCallback(frame);
        }

        return true;
    }

    uint32_t fourcc() const {
        // https://chromium.googlesource.com/libyuv/libyuv/+/refs/heads/master/include/libyuv/video_common.h
        QVideoFrame::PixelFormat format = surfaceFormat().pixelFormat();
        switch (format) {
        case QVideoFrame::Format_RGB24:
            return fourcc_from_string('r', 'a', 'w', ' ');
        case QVideoFrame::Format_Jpeg:
            return fourcc_from_string('M', 'J', 'P', 'G');
        default:
            return 0;
        }
    }

    QSize getResolution() const { return nativeResolution(); }
    void setResolution(const QSize &resolution) { setNativeResolution(resolution); }
    void setCalback(std::function<void(const QVideoFrame &frame)> cb) { mCallback = cb; }

    // helpers
    static inline uint32_t fourcc_from_string(const char &a, const char &b, const char &c,
                                              const char &d) {
        return ((static_cast<uint32_t>(a) << 0) | (static_cast<uint32_t>(b) << 8) |
                (static_cast<uint32_t>(c) << 16) | (static_cast<uint32_t>(d) << 24));
    }
    static inline std::string fourcc_to_string(uint32_t fourcc) {
        char str[4];
        std::strncpy(str, reinterpret_cast<char *>(&fourcc), 4);
        return std::string(str);
    }

private:
    std::function<void(const QVideoFrame &frame)> mCallback;
};

class QCameraCaptureStream : public ICaptureStream {
public:
    QCameraCaptureStream();
    virtual ~QCameraCaptureStream() override;

    bool isOpen() override { return mCamera.status() == QCamera::LoadedStatus; }

    bool open() override { return mCamera.status() == QCamera::LoadedStatus; }
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
    QSize getResolution();
    bool setFourcc(const uint32_t &fourcc) {
        (void)fourcc;
        return false;
    }
    bool setResolution(const QSize &res);

    // helpers
    static inline uint32_t fourcc_from_string(const char &a, const char &b, const char &c,
                                              const char &d) {
        return ((static_cast<uint32_t>(a) << 0) | (static_cast<uint32_t>(b) << 8) |
                (static_cast<uint32_t>(c) << 16) | (static_cast<uint32_t>(d) << 24));
    }
    static inline std::string fourcc_to_string(uint32_t fourcc) {
        char str[4];
        std::strncpy(str, reinterpret_cast<char *>(&fourcc), 4);
        return std::string(str);
    }

private:
    /// internal surface for Qt abstraction
    QCameraCaptureSurface mSurface;

    /// camera capture engine
    QCamera mCamera;

    /// surface frame buffer
    QVideoFrame mFrame;
    std::mutex mFrameLock;

    /// copy buffer
    ICaptureStream::Buffer mBuffer;
};

#endif // QCAMERA_CAPTURE_STREAM_H