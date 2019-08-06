#include <cstdlib>
#include <cstring>

#include "seraphim/core/fourcc.h"
#include "seraphim/core/image_buffer.h"
#include "seraphim/core/image_buffer_converter.h"

using namespace sph::core;

ImageBuffer::ImageBuffer() {
    m_data_owned = false;
    reset();
}

ImageBuffer::~ImageBuffer() {
    reset();
}

void ImageBuffer::reset() {
    if (m_data_owned) {
        // release allocated memory
        delete[] m_data;
    }

    m_data = nullptr;
    m_size = 0;
    m_data_owned = false;
    m_format = {};
}

ImageBuffer::Pixelformat ImageBuffer::as_pixelformat(const uint32_t &fourcc) {
    switch (fourcc) {
    case ::fourcc('B', 'G', 'R', '3'):
        return Pixelformat::BGR24;
    case ::fourcc('B', 'G', 'R', '4'):
        return Pixelformat::BGR32;
    case ::fourcc('R', 'G', 'B', '3'):
        return Pixelformat::RGB24;
    case ::fourcc('R', 'G', 'B', '4'):
        return Pixelformat::RGB32;
    case ::fourcc('Y', '1', '6', ' '):
        return Pixelformat::Y16;
    default:
        return Pixelformat::UNKNOWN;
    }
}

bool ImageBuffer::load(unsigned char *src, const Format &fmt) {
    size_t datalen;

    // determine the 1D data buffer length
    switch (fmt.pixfmt) {
    case Pixelformat::BGR24:
    case Pixelformat::RGB24:
        datalen = fmt.height * (fmt.width + fmt.padding) * 3 /* bpp / 8 */;
        break;
    case Pixelformat::BGR32:
    case Pixelformat::RGB32:
        datalen = fmt.height * (fmt.width + fmt.padding) * 4 /* bpp / 8 */;
        break;
    case Pixelformat::Y16:
        datalen = fmt.height * (fmt.width + fmt.padding) * 2 /* bpp / 8 */;
        break;
    default:
        return false;
    }

    // clear old data
    reset();

    std::memcpy(m_data, src, datalen);
    m_size = datalen;
    m_data_owned = true;
    m_format = fmt;

    return true;
}

bool ImageBuffer::load(unsigned char *src, const ImageBufferConverter::SourceFormat &src_fmt,
                       const Pixelformat &pixfmt) {
    size_t dst_size = 0;
    Format fmt = {};
    ImageBufferConverter::TargetFormat dst_fmt = {};

    fmt.width = src_fmt.width;
    fmt.height = src_fmt.height;
    fmt.padding = src_fmt.padding;
    fmt.pixfmt = ImageBuffer::Pixelformat::UNKNOWN;

    dst_fmt.padding = 0;

    // check whether conversion is necessary
    switch (src_fmt.fourcc) {
    case fourcc('B', 'G', 'R', '3'):
        fmt.pixfmt = Pixelformat::BGR24;
        break;
    case fourcc('B', 'G', 'R', '4'):
        fmt.pixfmt = Pixelformat::BGR32;
        break;
    case fourcc('R', 'G', 'B', '3'):
        fmt.pixfmt = Pixelformat::RGB24;
        break;
    case fourcc('R', 'G', 'B', '4'):
        fmt.pixfmt = Pixelformat::RGB32;
        break;
    case fourcc('Y', '1', '6', ' '):
        fmt.pixfmt = Pixelformat::Y16;
        break;
    default:
        break;
    }

    // determine the format to convert to
    switch (pixfmt) {
    case Pixelformat::BGR24:
        dst_fmt.fourcc = fourcc('B', 'G', 'R', '3');
        break;
    case Pixelformat::BGR32:
        dst_fmt.fourcc = fourcc('B', 'G', 'R', '4');
        break;
    case Pixelformat::RGB24:
        dst_fmt.fourcc = fourcc('R', 'G', 'B', '3');
        break;
    case Pixelformat::RGB32:
        dst_fmt.fourcc = fourcc('R', 'G', 'B', '4');
        break;
    case Pixelformat::Y16:
        dst_fmt.fourcc = fourcc('Y', '1', '6', ' ');
        break;
    default:
        return false;
    }

    if (fmt.pixfmt != Pixelformat::UNKNOWN) {
        // we know the format, so load the data right away
        return load(src, fmt);
    }

    // clear old data
    reset();

    if (fmt.pixfmt == Pixelformat::UNKNOWN) {
        // looks like we need to convert the buffer
        dst_size = ImageBufferConverter::Instance().convert(&src, src_fmt, &m_data, dst_fmt);
        if (dst_size == 0) {
            return false;
        }
    }

    m_size = dst_size;
    m_data_owned = true;
    m_format = fmt;

    return true;
}

bool ImageBuffer::assign(unsigned char *src, const Format &fmt, const bool &ownership) {
    size_t datalen;

    // determine the 1D data buffer length
    switch (fmt.pixfmt) {
    case Pixelformat::BGR24:
    case Pixelformat::RGB24:
        datalen = fmt.height * (fmt.width + fmt.padding) * 3 /* bpp / 8 */;
        break;
    case Pixelformat::BGR32:
    case Pixelformat::RGB32:
        datalen = fmt.height * (fmt.width + fmt.padding) * 4 /* bpp / 8 */;
        break;
    case Pixelformat::Y16:
        datalen = fmt.height * (fmt.width + fmt.padding) * 2 /* bpp / 8 */;
        break;
    default:
        return false;
    }

    // clear old data
    reset();

    m_data = src;
    m_size = datalen;
    m_data_owned = ownership;
    m_format = fmt;

    return true;
}

unsigned char *ImageBuffer::scanline(const uint32_t &y) const {
    size_t offset = y * (m_format.width + m_format.padding);

    if (offset > m_size) {
        return nullptr;
    }

    return m_data + offset;
}

unsigned char *ImageBuffer::pixel(const uint32_t &x, const uint32_t &y) const {
    unsigned char *scanline_ = scanline(y);
    size_t offset;

    if (scanline_ == nullptr) {
        return nullptr;
    }

    switch (m_format.pixfmt) {
    case Pixelformat::BGR24:
    case Pixelformat::RGB24:
        /* each pixel is three bytes */
        offset = x * 3;
        break;
    case Pixelformat::BGR32:
    case Pixelformat::RGB32:
        /* each pixel is four bytes */
        offset = x * 4;
        break;
    case Pixelformat::Y16:
        /* each pixel is two bytes */
        offset = x * 2;
        break;
    default:
        return nullptr;
    }

    if (offset > m_size) {
        // asked for an invalid pixel
        return nullptr;
    }

    return scanline_ + offset;
}

bool ImageBuffer::convert(const Pixelformat &target) {
    unsigned char *dst = nullptr;
    ImageBufferConverter::SourceFormat src_fmt = {};
    ImageBufferConverter::TargetFormat dst_fmt = {};

    if (m_format.pixfmt == target) {
        return true;
    }

    src_fmt.width = m_format.width;
    src_fmt.height = m_format.height;
    src_fmt.padding = m_format.padding;

    dst_fmt.padding = 0;

    switch (m_format.pixfmt) {
    case Pixelformat::BGR24:
        src_fmt.fourcc = fourcc('B', 'G', 'R', '3');
        break;
    case Pixelformat::BGR32:
        src_fmt.fourcc = fourcc('B', 'G', 'R', '4');
        break;
    case Pixelformat::RGB24:
        src_fmt.fourcc = fourcc('R', 'G', 'B', '3');
        break;
    case Pixelformat::RGB32:
        src_fmt.fourcc = fourcc('R', 'G', 'B', '4');
        break;
    case Pixelformat::Y16:
        src_fmt.fourcc = fourcc('Y', '1', '6', ' ');
        break;
    default:
        return false;
    }

    switch (target) {
    case Pixelformat::BGR24:
        dst_fmt.fourcc = fourcc('B', 'G', 'R', '3');
        break;
    case Pixelformat::BGR32:
        dst_fmt.fourcc = fourcc('B', 'G', 'R', '4');
        break;
    case Pixelformat::RGB24:
        dst_fmt.fourcc = fourcc('R', 'G', 'B', '3');
        break;
    case Pixelformat::RGB32:
        dst_fmt.fourcc = fourcc('R', 'G', 'B', '4');
        break;
    case Pixelformat::Y16:
        dst_fmt.fourcc = fourcc('Y', '1', '6', ' ');
        break;
    default:
        return false;
    }

    bool converted = false;
    if (m_data_owned) {
        // try to convert in-place first
        dst = m_data;
        m_size = ImageBufferConverter::Instance().convert(&m_data, src_fmt, &dst, dst_fmt);
        converted = m_size > 0;
    }

    if (!converted) {
        // try to convert by allocating a new buffer
        dst = nullptr;
        m_size = ImageBufferConverter::Instance().convert(&m_data, src_fmt, &dst, dst_fmt);
        converted = m_size > 0;
    }

    if (!converted) {
        // looks like we ran out of options
        reset();
        return false;
    }

    // at this point the new buffer is available
    // swap buffers if necessary
    if (m_data != dst) {
        if (m_data_owned) {
            // release allocated memory
            delete[] m_data;
        }
        m_data = dst;
        m_data_owned = true;
    }
    m_format.padding = dst_fmt.padding;
    m_format.pixfmt = target;

    return true;
}
