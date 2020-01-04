#ifndef QCAMERA_CAPTURE_STREAM_HPP
#define QCAMERA_CAPTURE_STREAM_HPP

#include <ICaptureStream/ICaptureStream.hpp>
#include <QtMultimedia/QAbstractVideoSurface>
#include <QtMultimedia/QCamera>
#include <QtMultimedia/QVideoSurfaceFormat>
#include <cstdint>
#include <cstring>
#include <mutex>

class QCameraCaptureSurface : public QAbstractVideoSurface {
    Q_OBJECT
public:
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
        QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const {
        Q_UNUSED(handleType);

        // Return the formats you will support
        return QList<QVideoFrame::PixelFormat>()
               << QVideoFrame::Format_RGB24 << QVideoFrame::Format_Jpeg;
    }

    bool present(const QVideoFrame &frame) {
        emit frameAvailable(frame);
        return true;
    }

    QSize getResolution() const { return nativeResolution(); }
    void setResolution(const QSize &resolution) { setNativeResolution(resolution); }

signals:
    void frameAvailable(const QVideoFrame &frame);
};

class QCameraCaptureStream : public QObject, public ICaptureStream {
    Q_OBJECT
public:
    QCameraCaptureStream();
    virtual ~QCameraCaptureStream() override;

    bool isOpen() override { return mCamera->status() == QCamera::LoadedStatus; }

    bool open() override;
    bool close() override;

    bool grab() override;
    bool retrieve(struct Buffer &buf) override;

    bool start() override {
        mCamera->start();
        return true;
    }
    bool stop() override {
        mCamera->stop();
        return true;
    }

    // convenience API
    bool open(const std::string &deviceName);

    // convenience API
    uint32_t getFourcc();
    QSize getResolution();
    bool setFourcc(const uint32_t &fourcc) {
        (void)fourcc;
        return false;
    }
    bool setResolution(const QSize &res);

    // helpers
    static inline uint32_t fourcc(const char &a, const char &b, const char &c, const char &d) {
        return ((static_cast<uint32_t>(a) << 0) | (static_cast<uint32_t>(b) << 8) |
                (static_cast<uint32_t>(c) << 16) | (static_cast<uint32_t>(d) << 24));
    }

signals:
    void bufferAvailable(const Buffer &buf);

public slots:
    void consumeFrame(const QVideoFrame &frame);

private:
    /// internal surface for Qt abstraction
    QCameraCaptureSurface mSurface;

    /// camera capture engine
    QCamera *mCamera;

    /// surface frame buffer
    QVideoFrame mFrame;
    std::mutex mFrameLock;

    /// copy buffer
    ICaptureStream::Buffer mBuffer;
};

#endif // QCAMERA_CAPTURE_STREAM_HPP
