#include <cstdlib>
#include <cstring>
#include <seraphim/core/image_buffer.h>

using namespace sph::core;

ImageBuffer::ImageBuffer() {
    m_data = nullptr;
    m_data_size = 0;
    m_data_owned = false;
}

ImageBuffer::~ImageBuffer() {
    if (m_data_owned) {
        std::free(m_data);
    }
}

template <typename Enumeration>
auto constexpr as_integer(Enumeration const value) ->
    typename std::underlying_type<Enumeration>::type {
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

ImageBuffer::Pixelformat ImageBuffer::as_pixelformat(const uint32_t &fourcc) {
    switch (fourcc) {
    case as_integer(ImageBuffer::Pixelformat::FMT_RGB24):
        return ImageBuffer::Pixelformat::FMT_RGB24;
    case as_integer(ImageBuffer::Pixelformat::FMT_YUYV):
        return ImageBuffer::Pixelformat::FMT_YUYV;
    case as_integer(ImageBuffer::Pixelformat::FMT_MJPG):
        return ImageBuffer::Pixelformat::FMT_MJPG;
    default:
        return ImageBuffer::Pixelformat::FMT_CUSTOM;
    }
}

bool ImageBuffer::copy_data(void *src, const size_t &len, const Pixelformat &fmt) {
    // clear old data
    if (m_data && m_data_owned) {
        std::free(m_data);
        m_data = nullptr;
    }

    std::memcpy(m_data, src, len);
    m_data_size = len;
    m_data_owned = true;
    m_pixelformat = fmt;

    return true;
}

bool ImageBuffer::wrap_data(void *src, const size_t &len, const Pixelformat &fmt,
                            const bool &ownership) {
    // clear old data
    if (m_data && m_data_owned) {
        std::free(m_data);
        m_data = nullptr;
    }

    m_data = src;
    m_data_size = len;
    m_data_owned = ownership;
    m_pixelformat = fmt;

    return true;
}
