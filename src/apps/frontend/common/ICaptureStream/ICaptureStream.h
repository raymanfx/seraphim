#ifndef I_CAPTURE_STREAM_H
#define I_CAPTURE_STREAM_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <thread>
#include <time.h>
#include <vector>

class ICaptureStream {
public:
    virtual ~ICaptureStream() = default;

    struct BufferFormat {
        uint32_t width;
        uint32_t height;
        uint32_t fourcc;
        uint32_t stride;
    };

    struct Buffer {
        void *start;
        size_t size;
        size_t bytesused;
        BufferFormat format;
    };

    // status query
    virtual bool isOpen() = 0;

    // device management
    virtual bool open() = 0;
    virtual bool close() = 0;

    // synchronous IO
    virtual bool grab() = 0;
    virtual bool retrieve(struct Buffer &buf) = 0;
    bool read(struct Buffer &buf) { return grab() && retrieve(buf); }

    // asynchronous IO
    // clients implement their own callback mechanisms
    /**
     * @brief Start the capture stream.
     * @return True if the async API is supported, false otherwise.
     */
    virtual bool start() = 0;
    /**
     * @brief Stop the capture stream.
     * @return True if the async API is supported, false otherwise.
     */
    virtual bool stop() = 0;
};

#endif // I_CAPTURE_STREAM_H
