#include <cstring>
#include <errno.h>
#include <iostream>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "v4l2_device.h"

static int xioctl(int fd, unsigned long request, void *arg) {
    int ret;

    do {
        ret = ioctl(fd, request, arg);
    } while (ret == -1 && errno == EINTR);

    return ret;
}

V4L2Device::~V4L2Device() {
    // stop active stream
    if (m_stream_active) {
        stop_stream();
    }

    // close the device handle
    if (m_fd >= 0) {
        ::close(m_fd);
    }
    m_initialized = false;

    if (!m_buffers.empty()) {
        release_buffers();
    }
}

bool V4L2Device::open(const std::string &path) {
    struct v4l2_capability cap = {};

    close();

    m_path = path;
    m_fd = ::open(m_path.c_str(), O_RDWR);
    if (m_fd == -1) {
        return false;
    }

    // there are a number of conditions that must be matched for a capture device
    if (ioctl(VIDIOC_QUERYCAP, &cap) == -1) {
        return false;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        return false;
    }

    m_initialized = true;
    return m_initialized;
}

bool V4L2Device::open(const int &index) {
    m_path = "/dev/video" + std::to_string(index);
    return open(m_path);
}

bool V4L2Device::close() {
    // stop active stream
    if (m_stream_active) {
        stop_stream();
    }

    if (m_fd >= 0) {
        if (::close(m_fd) == -1) {
            return false;
        }
    }

    m_fd = -1;
    m_initialized = false;
    return true;
}

bool V4L2Device::reset() {
    close();
    return open(m_path);
}

bool V4L2Device::init_buffers(const int &iomethod, const uint32_t &num_buffers) {
    bool ret = true;
    struct v4l2_capability cap = {};

    if (ioctl(VIDIOC_QUERYCAP, &cap) == -1) {
        return false;
    }

    if (!m_buffers.empty()) {
        release_buffers();
    }

    m_buffers.resize(num_buffers);

    switch (iomethod) {
    case IO_METHOD_READ:
        ret = cap.capabilities & V4L2_CAP_READWRITE;
        if (ret)
            ret = init_buffers_read();
        break;
    case IO_METHOD_MMAP:
        ret = cap.capabilities & V4L2_CAP_STREAMING;
        if (ret)
            ret = init_buffers_mmap();
        break;
    default:
        ret = false;
        break;
    }

    if (!ret) {
        m_buffers.clear();
        m_iomethod = -1;
    } else {
        m_iomethod = iomethod;
    }

    return ret;
}

bool V4L2Device::init_buffers_mmap() {
    struct v4l2_requestbuffers req = {};

    req.count = static_cast<uint32_t>(m_buffers.size());
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(VIDIOC_REQBUFS, &req) == -1) {
        return false;
    }

    for (uint32_t i = 0; i < m_buffers.size(); i++) {
        struct v4l2_buffer buf = {};

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(VIDIOC_QUERYBUF, &buf) == -1) {
            return false;
        }

        m_buffers[i].start = mmap(nullptr, buf.length, PROT_READ, MAP_SHARED, m_fd, buf.m.offset);
        if (m_buffers[i].start == MAP_FAILED) {
            return false;
        }

        m_buffers[i].length = buf.length;
    }

    return true;
}

bool V4L2Device::init_buffers_read() {
    bool ret = true;
    uint32_t min_buf_sz;
    struct v4l2_format fmt = {};

    // get format to determine buffer size
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(VIDIOC_G_FMT, &fmt) == -1) {
        return false;
    }

    // buggy driver paranoia
    min_buf_sz = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min_buf_sz) {
        fmt.fmt.pix.bytesperline = min_buf_sz;
    }
    min_buf_sz = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min_buf_sz) {
        fmt.fmt.pix.sizeimage = min_buf_sz;
    }

    // allocate buffers
    for (size_t i = 0; i < m_buffers.size(); i++) {
        m_buffers[i].start = std::malloc(fmt.fmt.pix.sizeimage);
        if (!m_buffers[i].start) {
            ret = false;
            break;
        }
        m_buffers[i].length = fmt.fmt.pix.sizeimage;
    }

    if (!ret) {
        // release memory on error
        for (size_t i = 0; i < m_buffers.size(); i++) {
            if (m_buffers[i].start) {
                std::free(m_buffers[i].start);
            }
        }
    }

    return ret;
}

bool V4L2Device::release_buffers() {
    bool ret;

    if (m_buffers.empty()) {
        return true;
    }

    switch (m_iomethod) {
    case IO_METHOD_READ:
        ret = release_buffers_read();
        break;
    case IO_METHOD_MMAP:
        ret = release_buffers_mmap();
        break;
    default:
        ret = false;
        break;
    }

    m_buffers.clear();
    return ret;
}

bool V4L2Device::release_buffers_mmap() {
    bool ret = true;
    struct v4l2_requestbuffers req = {};

    req.count = 0;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    for (size_t i = 0; i < m_buffers.size(); i++) {
        if (!m_buffers[i].start) {
            continue;
        }

        if (munmap(m_buffers[i].start, m_buffers[i].length) == -1) {
            ret = false;
        }
    }

    return ret;
}

bool V4L2Device::release_buffers_read() {
    for (size_t i = 0; i < m_buffers.size(); i++) {
        if (m_buffers[i].start) {
            std::free(m_buffers[i].start);
        }
    }

    return true;
}

int V4L2Device::ioctl(unsigned long request, void *arg) {
    int rc;

    rc = xioctl(m_fd, request, arg);
    if (rc == -1 && errno == ENODEV) {
        m_initialized = false;
    }

    return rc;
}

void V4L2Device::print_capabilities() {
    struct v4l2_capability caps = {};

    ioctl(VIDIOC_QUERYCAP, &caps);
    std::cout << "Card:        \"" << caps.card << "\"" << std::endl
              << "Driver:      \"" << caps.driver << "\"" << std::endl
              << "Bus:         \"" << caps.bus_info << "\"" << std::endl
              << "Version:     \"" << ((caps.version >> 16) & 0xFF) << "."
              << ((caps.version >> 8) & 0xFF) << "." << (caps.version & 0xFF) << "\"" << std::endl
              << "Device caps: \"" << caps.device_caps << std::endl;
}

void V4L2Device::print_formats() {
    struct v4l2_fmtdesc fmtdesc = {};
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    char fourcc[5];
    char c, e;
    printf(" Format | CE | Description\n"
           "---------------------------\n");
    while (ioctl(VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
        strncpy(fourcc, reinterpret_cast<char *>(&fmtdesc.pixelformat), 4);
        c = fmtdesc.flags & 1 ? 'C' : ' ';
        e = fmtdesc.flags & 2 ? 'E' : ' ';
        printf("  %s  | %c%c | %s\n", fourcc, c, e, fmtdesc.description);
        fmtdesc.index++;
    }
}

bool V4L2Device::start_stream() {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    uint32_t buffers = 4;

    if (m_buffers.empty()) {
        // try to initialize buffers
        for (uint32_t i = buffers; i > 0; i--) {
            if (init_buffers(IO_METHOD_MMAP, i)) {
                break;
            }
        }
    }

    if (m_buffers.empty()) {
        // cannot recover at this point, the user should initialize the buffers manually
        return false;
    }

    if (ioctl(VIDIOC_STREAMON, &type) == -1) {
        return false;
    }

    m_stream_active = true;
    return true;
}

bool V4L2Device::stop_stream() {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(VIDIOC_STREAMOFF, &type) == -1) {
        return false;
    }

    m_stream_active = false;
    return true;
}

bool V4L2Device::grab() {
    struct v4l2_buffer v4l2_buf = {};
    static uint32_t buffer_index = 0;

    if (!m_stream_active) {
        // enable the stream (convenience API handling)
        if (!start_stream()) {
            return false;
        }
    }

    if (m_buffers.empty()) {
        return false;
    }

    switch (m_iomethod) {
    case IO_METHOD_READ:
        /* nothing to do */
        break;
    case IO_METHOD_MMAP:
        v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4l2_buf.memory = V4L2_MEMORY_MMAP;
        v4l2_buf.index = buffer_index;

        if (ioctl(VIDIOC_QBUF, &v4l2_buf) == -1) {
            return false;
        }

        break;
    default:
        break;
    }

    buffer_index = (buffer_index + 1) % m_buffers.size();
    return true;
}

bool V4L2Device::retrieve(struct Buffer &buf) {
    ssize_t read;
    struct v4l2_buffer v4l2_buf = {};
    struct timespec now = {};
    static uint32_t buffer_index = 0;
    static uint32_t frame = 0;

    if (m_buffers.empty()) {
        return false;
    }

    switch (m_iomethod) {
    case IO_METHOD_READ:
        read = ::read(m_fd, m_buffers[buffer_index].start, m_buffers[buffer_index].length);
        if (read == -1) {
            return false;
        }

        // emulate buffer properties
        clock_gettime(CLOCK_MONOTONIC, &now);
        m_buffers[buffer_index].bytesused = read;
        m_buffers[buffer_index].sequence = frame;
        m_buffers[buffer_index].timestamp.tv_sec = now.tv_sec;
        m_buffers[buffer_index].timestamp.tv_usec = now.tv_nsec / 1000;
        frame++;
        break;
    case IO_METHOD_MMAP:
        v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4l2_buf.memory = V4L2_MEMORY_MMAP;
        v4l2_buf.index = buffer_index;

        if (ioctl(VIDIOC_DQBUF, &v4l2_buf) == -1) {
            return false;
        }

        if (v4l2_buf.bytesused == 0) {
            // TODO: maybe the camera was unplugged?
            // Either way we failed to capture a frame and should propagate this
            return false;
        }

        m_buffers[buffer_index].bytesused = v4l2_buf.bytesused;
        m_buffers[buffer_index].sequence = v4l2_buf.sequence;
        m_buffers[buffer_index].timestamp = v4l2_buf.timestamp;
        break;
    default:
        break;
    }

    buf = m_buffers[buffer_index];
    buffer_index = (buffer_index + 1) % m_buffers.size();

    return true;
}

bool V4L2Device::read(struct Buffer &buf) {
    return grab() && retrieve(buf);
}
