#ifndef V4L2_DEVICE_H
#define V4L2_DEVICE_H

#include <fcntl.h>
#include <linux/videodev2.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

class V4L2Device {
public:
    V4L2Device();
    ~V4L2Device();

    enum io_method { IO_METHOD_READ, IO_METHOD_MMAP };

    struct Buffer {
        /* Address of the buffer set by mmap */
        void *start;
        /* Size of the buffer (not the payload) in bytes */
        uint32_t length;
        /* Size of the payload in bytes */
        uint32_t bytesused;
        /* Set by the driver, counting the frames (not fields!) in sequence */
        uint32_t sequence;
        /*
         * Time when the first data byte was captured, as returned by the clock_gettime() function
         * for the relevant clock id; see V4L2_BUF_FLAG_TIMESTAMP_*
         */
        struct timeval timestamp;
    };

    bool open(const std::string &path);
    bool open(const int &index);
    bool close();
    bool reset();

    bool init_buffers(const int &iomethod, const uint32_t &num_buffers);
    bool release_buffers();

    int ioctl(unsigned long request, void *arg);

    void print_capabilities();
    void print_formats();

    // stream management
    bool start_stream();
    bool stop_stream();

    // grab the next frame in the pipe
    bool grab();
    // retrieve the current frame
    bool retrieve(struct Buffer &buf);
    // grab() and retrieve() to query a new frame
    bool read(struct Buffer &buf);

    // state checking
    bool initialized() const { return m_initialized; }
    bool active() const { return m_stream_active; }

private:
    int m_fd;
    std::string m_path;
    int m_iomethod;

    uint32_t m_num_buffers;
    struct Buffer *m_buffers;

    bool m_initialized;
    bool m_stream_active;

    // buffer management
    bool init_buffers_mmap();
    bool init_buffers_read();
    bool release_buffers_mmap();
    bool release_buffers_read();
};

#endif // V4L2_DEVICE_H
