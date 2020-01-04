#ifndef V4L2_DEVICE_HPP
#define V4L2_DEVICE_HPP

#include <fcntl.h>
#include <linux/videodev2.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

/**
 * @brief Video for Linux 2 (v4l2) device abstraction.
 *
 * Wraps v4l2 capture device handling in a C++ API.
 * Buffer creation is abstracted so the caller does not have to worry about
 * device hardware handling here.
 *
 * Reading a frame is a twofold process consisting of @ref grab and @ref retrieve
 * APIs. The former prepares a new buffer on the device (the actual capturing) while
 * the latter actually "retrieves" the buffer for userspace.
 */
class V4L2Device {
public:
    V4L2Device();
    ~V4L2Device();

    /**
     * @brief I/O device access method.
     */
    enum io_method {
        /// read() API similar to reading from a file
        IO_METHOD_READ,
        /// map device memory into userspace, allowing for zero-copy operations
        IO_METHOD_MMAP
    };

    /**
     * @brief Capture buffer representation and metadata.
     */
    struct Buffer {
        /// Address of the buffer set by mmap
        void *start;
        /// Size of the buffer (not the payload) in bytes
        uint32_t length;
        /// Size of the payload in bytes
        uint32_t bytesused;
        /// Set by the driver, counting the frames (not fields!) in sequence
        uint32_t sequence;
        /// Time when the first data byte was captured, as returned by the clock_gettime() function
        /// for the relevant clock id; see V4L2_BUF_FLAG_TIMESTAMP_*
        struct timeval timestamp;
    };

    /**
     * @brief Open the device provided by a kernel driver.
     * @param path Absolute path of the device, e.g. /dev/video0.
     * @return True on success, false otherwise.
     */
    bool open(const std::string &path);
    /**
     * @brief Open the device provided by a kernel driver.
     * @param index Index of the device, e.g. 0 -> /dev/video0.
     * @return True on success, false otherwise.
     */
    bool open(const int &index);
    /**
     * @brief Close the file descriptor of a device and release any associated resources.
     * @return True on success, false otherwise.
     */
    bool close();
    /**
     * @brief Reset the file descriptor of the device, useful when a device (e.g. USB camera) was
     * unplugged.
     * @return True on success, false otherwise.
     */
    bool reset();

    /**
     * @brief Initialize a devices' buffers.
     * @param iomethod I/O method, one of @ref io_method.
     * @param num_buffers Number of buffers to allocate on the device.
     * @return True on success, false otherwise.
     */
    bool init_buffers(const int &iomethod, const uint32_t &num_buffers);
    /**
     * @brief Free a devices' buffers.
     * @return True on success, false otherwise.
     */
    bool release_buffers();

    /**
     * @brief Execute an ioctl for a device.
     * @param request ioctl request (e.g. VIDIOC_QBUF).
     * @param arg ioctl argument (e.g. a v4l2_buf struct pointer).
     * @return True on success, false otherwise.
     */
    int ioctl(unsigned long request, void *arg);

    // not documenting these as they should go away at some point?
    void print_capabilities();
    void print_formats();

    /**
     * @brief Start the streaming mode. Has to be done before capturing any frames.
     * @return True on success, false otherwise.
     */
    bool start_stream();
    /**
     * @brief Stop the streaming mode.
     * @return True on success, false otherwise.
     */
    bool stop_stream();

    /**
     * @brief Prepare the next frame in the image capture pipeline. Usually, this will cause the
     * camera to capture an image and save it into a local device memory buffer.
     * @return True on success, false otherwise.
     */
    bool grab();
    /**
     * @brief Grab a buffer from the device memory space and make it accessible to userspace.
     * Depending on the I/O method chosen (see @ref io_method), this will either copy the device
     * memory or map it.
     * @param buf The output buffer representation.
     * @return True on success, false otherwise.
     */
    bool retrieve(struct Buffer &buf);
    /**
     * @brief Performs @ref grab and @ref retrive sequentially.
     * @param buf The output buffer representation.
     * @return True on success, false otherwise.
     */
    bool read(struct Buffer &buf);

    /**
     * @brief Whether the device has been initialized successfully (i.e. file descriptor is open and
     * basic v4l2 capabilities are matched).
     * @return True on success, false otherwise.
     */
    bool initialized() const { return m_initialized; }
    /**
     * @brief Whether the device is in streaming mode.
     * @return True if in streaming mode, false otherwise.
     */
    bool active() const { return m_stream_active; }

private:
    // file descriptor of the device
    int m_fd = -1;
    // absolute path of the device in the Linux pseudo-FS (e.g. /dev or /sys).
    std::string m_path = "";
    // I/O method for accessing buffers (see @ref io_method).
    int m_iomethod = -1;

    // staging area for buffers that are returned to callers of @ref retrieve.
    std::vector<struct Buffer> m_buffers;

    bool m_initialized = false;
    bool m_stream_active = false;

    // buffer management
    bool init_buffers_mmap();
    bool init_buffers_read();
    bool release_buffers_mmap();
    bool release_buffers_read();
};

#endif // V4L2_DEVICE_HPP
