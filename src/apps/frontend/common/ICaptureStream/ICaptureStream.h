#ifndef I_CAPTURE_STREAM_H
#define I_CAPTURE_STREAM_H

#include <atomic>
#include <cstdint>
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

    // IO
    virtual bool grab() = 0;
    virtual bool retrieve(struct Buffer &buf) = 0;
    bool read(struct Buffer &buf) { return grab() && retrieve(buf); }
};

#endif // I_CAPTURE_STREAM_H
