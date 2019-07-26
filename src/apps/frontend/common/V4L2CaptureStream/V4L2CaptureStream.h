#ifndef V4L2_CAPTURE_STREAM_H
#define V4L2_CAPTURE_STREAM_H

#include <ICaptureStream/ICaptureStream.h>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <functional>
#include <thread>
#include <time.h>
#include <vector>

#include "v4l2_device.h"

class V4L2CaptureStream : public ICaptureStream {
public:
    V4L2CaptureStream();
    ~V4L2CaptureStream() override;

    bool isOpen() override { return mDevice.initialized(); }

    bool open() override;
    bool close() override { return mDevice.close(); }

    bool grab() override { return mDevice.grab(); }
    bool retrieve(struct Buffer &buf) override;

    bool start() override;
    bool stop() override;
    void setFrameCallback(std::function<void(const Buffer &)> fn) { mFrameCallback = fn; }

    // implement passthrough methods
    bool open(const std::string &path);
    bool open(const int &index);
    bool reset() { return mDevice.reset(); }
    bool init_buffers(const int &iomethod, const uint32_t &num_buffers) {
        return mDevice.init_buffers(iomethod, num_buffers);
    }
    bool release_buffers() { return mDevice.release_buffers(); }

    // convenience API
    uint32_t getFourcc() { return mFourcc; }
    uint32_t getWidth() { return mWidth; }
    uint32_t getHeight() { return mHeight; }
    bool setFourcc(const uint32_t &fourcc);
    bool setResolution(const uint32_t &width, const uint32_t &height);

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
    /// the actual capture device abstraction
    V4L2Device mDevice;

    /// internal path to the capture device
    std::string mDevicePath;

    /// capture thread for async I/O
    std::thread mCaptureThread;
    std::atomic<bool> mCaptureActive;
    /// callback for async capture notifications
    std::function<void(const Buffer &)> mFrameCallback;

    /// capture format
    uint32_t mFourcc;
    uint32_t mWidth;
    uint32_t mHeight;
};

#endif // V4L2_CAPTURE_STREAM_H
