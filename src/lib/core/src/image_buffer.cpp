#include <cstdlib>
#include <cstring>

#include "seraphim/core/fourcc.h"
#include "seraphim/core/image_buffer.h"
#include "seraphim/core/image_buffer_converter.h"

using namespace sph::core;

static bool validate_format(ImageBuffer::Format &fmt) {
    if (fmt.width == 0 || fmt.height == 0) {
        return false;
    }

    if (fmt.stride > 0 && fmt.stride < fmt.width) {
        return false;
    }

    if (fmt.stride == 0) {
        switch (fmt.pixfmt) {
        case ImageBuffer::Pixelformat::BGR24:
        case ImageBuffer::Pixelformat::RGB24:
            fmt.stride = fmt.width * 3;
            break;
        case ImageBuffer::Pixelformat::BGR32:
        case ImageBuffer::Pixelformat::RGB32:
            fmt.stride = fmt.width * 4;
            break;
        case ImageBuffer::Pixelformat::Y8:
            fmt.stride = fmt.width * 1;
            break;
        case ImageBuffer::Pixelformat::Y16:
            fmt.stride = fmt.width * 2;
            break;
        default:
            return false;
        }
    }

    return true;
}

ImageBuffer::ImageBuffer() {
    clear();
}

ImageBuffer::ImageBuffer(const ImageBuffer &buf) {
    // create a shallow copy
    m_data = const_cast<unsigned char *>(buf.data());
    m_data_buffer.clear();
    m_format = buf.format();
}

ImageBuffer::~ImageBuffer() {
    clear();
}

void ImageBuffer::clear() {
    m_data = nullptr;
    m_data_buffer.clear();
    m_format = {};
}

void ImageBuffer::shrink() {
    m_data_buffer.shrink_to_fit();
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
    case ::fourcc('G', 'R', 'E', 'Y'):
        return Pixelformat::Y8;
    case ::fourcc('Y', '1', '6', ' '):
        return Pixelformat::Y16;
    default:
        return Pixelformat::UNKNOWN;
    }
}

bool ImageBuffer::load(const ImageBuffer &buf) {
    const unsigned char *src = buf.data();
    size_t src_len = buf.size();
    Format src_fmt = buf.format();

    if (!validate_format(src_fmt)) {
        return false;
    }

    // do not attempt to copy ourselves
    if (this == &buf && !m_data_buffer.empty()) {
        return true;
    }

    // clear old data
    clear();

    m_data_buffer.assign(src, src + src_len);
    m_data = m_data_buffer.data();
    m_format = src_fmt;

    return true;
}

bool ImageBuffer::load(const unsigned char *src, const Format &fmt) {
    size_t src_len;
    Format src_fmt = fmt;

    if (!validate_format(src_fmt)) {
        return false;
    }

    // clear old data
    clear();

    src_len = src_fmt.height * src_fmt.stride;

    m_data_buffer.assign(src, src + src_len);
    m_data = m_data_buffer.data();
    m_format = src_fmt;

    return true;
}

bool ImageBuffer::load(const unsigned char *src, const ImageBufferConverter::SourceFormat &src_fmt,
                       const Pixelformat &pixfmt) {
    Format fmt = {};
    ImageBufferConverter::TargetFormat dst_fmt = {};
    size_t dst_padding;

    fmt.width = src_fmt.width;
    fmt.height = src_fmt.height;
    fmt.pixfmt = ImageBuffer::Pixelformat::UNKNOWN;

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
    case fourcc('G', 'R', 'E', 'Y'):
        fmt.pixfmt = Pixelformat::Y8;
        break;
    case fourcc('Y', '1', '6', ' '):
        fmt.pixfmt = Pixelformat::Y16;
        break;
    default:
        break;
    }

    if (!validate_format(fmt)) {
        return false;
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
    case Pixelformat::Y8:
        dst_fmt.fourcc = fourcc('G', 'R', 'E', 'Y');
        break;
    case Pixelformat::Y16:
        dst_fmt.fourcc = fourcc('Y', '1', '6', ' ');
        break;
    default:
        return false;
    }

    // always align on word boundaries
    dst_fmt.alignment = 4;

    // the general formula is:
    // padding = (ALIGN - (LENGTH % ALIGN)) % ALIGN
    dst_padding = (dst_fmt.alignment - (src_fmt.stride % dst_fmt.alignment)) % dst_fmt.alignment;

    // calculate the target stride
    fmt.stride = fmt.width + dst_padding;

    if (fmt.pixfmt != Pixelformat::UNKNOWN) {
        // we know the format, so load the data right away
        return load(src, fmt);
    }

    // clear old data
    clear();

    // looks like we need to convert the buffer
    bool converted = false;
    unsigned char *src_ = const_cast<unsigned char *>(src);
    converted = ImageBufferConverter::Instance().convert(src_, src_fmt, m_data_buffer, dst_fmt);
    if (!converted) {
        // something went wrong in the converter code
        clear();
        return false;
    }

    m_format = fmt;

    return true;
}

bool ImageBuffer::assign(unsigned char *src, const Format &fmt) {
    Format src_fmt = fmt;

    if (!validate_format(src_fmt)) {
        return false;
    }

    // clear old data
    clear();

    m_data = src;
    m_format = src_fmt;

    return true;
}

unsigned char *ImageBuffer::scanline(const uint32_t &y) const {
    size_t offset = y * m_format.stride;

    if (offset > size()) {
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
    case Pixelformat::Y8:
        /* each pixel is one byte */
        offset = x;
        break;
    case Pixelformat::Y16:
        /* each pixel is two bytes */
        offset = x * 2;
        break;
    default:
        return nullptr;
    }

    if (offset > size()) {
        // asked for an invalid pixel
        return nullptr;
    }

    return scanline_ + offset;
}

bool ImageBuffer::convert(const Pixelformat &target) {
    size_t dst_padding;
    size_t dst_pixel_size;
    ImageBufferConverter::SourceFormat src_fmt = {};
    ImageBufferConverter::TargetFormat dst_fmt = {};

    if (m_format.pixfmt == target) {
        return true;
    }

    src_fmt.width = m_format.width;
    src_fmt.height = m_format.height;
    src_fmt.stride = m_format.stride;
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
    case Pixelformat::Y8:
        src_fmt.fourcc = fourcc('G', 'R', 'E', 'Y');
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
        dst_pixel_size = 3;
        break;
    case Pixelformat::BGR32:
        dst_fmt.fourcc = fourcc('B', 'G', 'R', '4');
        dst_pixel_size = 4;
        break;
    case Pixelformat::RGB24:
        dst_fmt.fourcc = fourcc('R', 'G', 'B', '3');
        dst_pixel_size = 3;
        break;
    case Pixelformat::RGB32:
        dst_fmt.fourcc = fourcc('R', 'G', 'B', '4');
        dst_pixel_size = 4;
        break;
    case Pixelformat::Y8:
        dst_fmt.fourcc = fourcc('G', 'R', 'E', 'Y');
        dst_pixel_size = 1;
        break;
    case Pixelformat::Y16:
        dst_fmt.fourcc = fourcc('Y', '1', '6', ' ');
        dst_pixel_size = 2;
        break;
    default:
        return false;
    }

    // always align on word boundaries
    dst_fmt.alignment = 4;

    // the general formula is:
    // padding = (ALIGN - (LENGTH % ALIGN)) % ALIGN
    dst_padding = (dst_fmt.alignment - (src_fmt.stride % dst_fmt.alignment)) % dst_fmt.alignment;

    bool converted = false;

    // try to convert in-place first
    converted = ImageBufferConverter::Instance().convert(m_data, src_fmt, m_data_buffer, dst_fmt);

    if (!converted) {
        // allocate a new buffer by first moving the contents to an alternative location
        std::vector<unsigned char> tmp;
        m_data_buffer.swap(tmp);
        converted =
            ImageBufferConverter::Instance().convert(tmp.data(), src_fmt, m_data_buffer, dst_fmt);
    }

    if (!converted) {
        // looks like we ran out of options
        clear();
        return false;
    }

    // at this point the new buffer is available
    m_data = m_data_buffer.data();
    m_format.width = src_fmt.width;
    m_format.height = src_fmt.height;
    m_format.pixfmt = target;
    m_format.stride = (src_fmt.width + dst_padding) * dst_pixel_size;

    return true;
}
