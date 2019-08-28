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
        fmt.stride = fmt.width * ImageBuffer::pixsize(fmt.pixfmt);
    }

    return fmt.stride > 0;
}

ImageBuffer::ImageBuffer(const ImageBuffer &buf) {
    clear();

    // create a shallow copy
    m_data = buf.data();
    m_format = buf.format();
}

ImageBuffer::~ImageBuffer() {
    clear();
}

void ImageBuffer::operator=(const ImageBuffer &buf) {
    clear();

    // create a shallow copy
    m_data = buf.data();
    m_format = buf.format();
}

void ImageBuffer::clear() {
    m_data = nullptr;
    m_data_buffer.clear();
    m_format = {};
}

void ImageBuffer::shrink() {
    m_data_buffer.shrink_to_fit();
}

ImageBuffer::Pixelformat ImageBuffer::fourcc2pixfmt(const uint32_t &fourcc) {
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

uint8_t ImageBuffer::pixsize(const Pixelformat &fmt) {
    switch (fmt) {
    case Pixelformat::BGR24:
    case Pixelformat::RGB24:
        return 3;
    case Pixelformat::BGR32:
    case Pixelformat::RGB32:
        return 4;
    case Pixelformat::Y8:
        return 1;
    case Pixelformat::Y16:
        return 2;
    default:
        return 0;
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
    if (this != &buf) {
        clear();
    }

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

bool ImageBuffer::load(const ImageBufferConverter::Source &src, const Pixelformat &pixfmt) {
    Format fmt = {};
    ImageBufferConverter::Source src_ = {};
    ImageBufferConverter::Target dst = {};
    size_t dst_padding;

    fmt.width = src.width;
    fmt.height = src.height;
    fmt.pixfmt = fourcc2pixfmt(src.fourcc);

    if (!validate_format(fmt)) {
        return false;
    }

    src_.buf = src.buf;
    src_.width = src.width;
    src_.height = src.height;
    src_.fourcc = src.fourcc;
    src_.stride = fmt.stride;

    dst.buf = m_data_buffer.data();
    dst.buf_len = m_data_buffer.size();
    dst.fourcc = pixfmt2fourcc(pixfmt);
    if (dst.fourcc == 0) {
        return false;
    }

    // always align on word boundaries
    dst.alignment = 4;

    // the general formula is:
    // padding = (ALIGN - (LENGTH % ALIGN)) % ALIGN
    dst_padding = (dst.alignment - (src_.stride % dst.alignment)) % dst.alignment;

    // calculate the target stride
    fmt.stride = fmt.width + dst_padding;

    if (fmt.pixfmt != Pixelformat::UNKNOWN) {
        // we know the format, so load the data right away
        return load(src_.buf, fmt);
    }

    // clear old data
    clear();

    // probe the target buffer size
    size_t dst_size = ImageBufferConverter::probe(src, dst);
    if (dst_size == 0) {
        return false;
    }

    // prepare target buffer
    if (dst_size > m_data_buffer.size()) {
        m_data_buffer.resize(dst_size);
        dst.buf = m_data_buffer.data();
        dst.buf_len = m_data_buffer.size();
    }

    // looks like we need to convert the buffer
    bool converted = ImageBufferConverter::Instance().convert(src_, dst);
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
    uint8_t pixsize;

    if (scanline_ == nullptr) {
        return nullptr;
    }

    pixsize = ImageBuffer::pixsize(m_format.pixfmt);
    if (pixsize == 0) {
        // invalid format
        return nullptr;
    }

    offset = x * pixsize;
    if (offset > size()) {
        // asked for an invalid pixel
        return nullptr;
    }

    return scanline_ + offset;
}

bool ImageBuffer::convert(const Pixelformat &target) {
    size_t dst_padding;
    size_t dst_pixel_size;
    ImageBufferConverter::Source src = {};
    ImageBufferConverter::Target dst = {};

    if (m_format.pixfmt == target) {
        return true;
    }

    src.buf = m_data;
    src.width = m_format.width;
    src.height = m_format.height;
    src.stride = m_format.stride;
    src.fourcc = pixfmt2fourcc(m_format.pixfmt);
    if (src.fourcc == 0) {
        return false;
    }

    dst.buf = m_data_buffer.data();
    dst.buf_len = m_data_buffer.size();
    dst.fourcc = pixfmt2fourcc(target);
    if (dst.fourcc == 0) {
        return false;
    }

    dst_pixel_size = ImageBuffer::pixsize(target);
    if (dst_pixel_size == 0) {
        return false;
    }

    // always align on word boundaries
    dst.alignment = 4;

    // the general formula is:
    // padding = (ALIGN - (LENGTH % ALIGN)) % ALIGN
    dst_padding = (dst.alignment - (src.stride % dst.alignment)) % dst.alignment;

    // probe the target buffer size
    size_t dst_size = ImageBufferConverter::probe(src, dst);
    if (dst_size == 0) {
        return false;
    }

    // prepare target buffer
    std::vector<unsigned char> tmp;
    if (dst_size > m_data_buffer.size()) {
        if (src.buf == dst.buf) {
            // backup the current buffer first so we don't lose the source
            m_data_buffer.swap(tmp);
            src.buf = tmp.data();
        }
        m_data_buffer.resize(dst_size);
        dst.buf = m_data_buffer.data();
        dst.buf_len = m_data_buffer.size();
    }

    bool converted = false;

    // try to convert in-place first
    converted = ImageBufferConverter::Instance().convert(src, dst);

    if (!converted) {
        // allocate a new buffer by first moving the contents to an alternative location
        std::vector<unsigned char> tmp;
        m_data_buffer.swap(tmp);
        src.buf = tmp.data();
        converted = ImageBufferConverter::Instance().convert(src, dst);
    }

    if (!converted) {
        // looks like we ran out of options
        clear();
        return false;
    }

    // at this point the new buffer is available
    m_data = m_data_buffer.data();
    m_format.width = src.width;
    m_format.height = src.height;
    m_format.pixfmt = target;
    m_format.stride = (src.width + dst_padding) * dst_pixel_size;

    return true;
}
