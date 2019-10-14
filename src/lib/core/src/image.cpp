#include <seraphim/core/image.h>

using namespace sph;

Image::Image(unsigned char *data, uint32_t width, uint32_t height, Pixelformat::Enum pixfmt,
             size_t stride)
    : m_width(width), m_height(height), m_pixfmt(pixfmt) {
    if (stride == 0) {
        stride = m_width * Pixelformat::bits(m_pixfmt) / 8;
    }
    m_buffer =
        Matrix<unsigned char>(data, m_height, m_width * Pixelformat::bits(m_pixfmt) / 8, stride);
}

uint32_t Image::channels() const {
    switch (m_pixfmt) {
    case Pixelformat::Enum::GRAY8:
    case Pixelformat::Enum::GRAY16:
        return 1;
    case Pixelformat::Enum::BGR24:
    case Pixelformat::Enum::BGR32:
    case Pixelformat::Enum::RGB24:
    case Pixelformat::Enum::RGB32:
        return 3;
    default:
        return 0;
    }
}

uint32_t Image::depth() const {
    return Pixelformat::bits(m_pixfmt);
}

void Image::clear() {
    m_width = 0;
    m_height = 0;
    m_buffer.clear();
}

bool Image::load(const ImageConverter::Source &src, Pixelformat::Enum pixfmt) {
    ImageConverter::Target dst = {};
    size_t dst_padding;
    size_t dst_stride;

    dst.buf = m_buffer.data();
    dst.buf_len = m_buffer.rows() * m_buffer.step();
    dst.fourcc = Pixelformat::fourcc(pixfmt);
    if (dst.fourcc == 0) {
        return false;
    }

    // always align on word boundaries
    dst.alignment = 4;

    // calculate the target stride
    dst_stride = src.width * Pixelformat::bits(pixfmt) / 8;

    // the general formula is:
    // padding = (ALIGN - (LENGTH % ALIGN)) % ALIGN
    dst_padding = (dst.alignment - (dst_stride % dst.alignment)) % dst.alignment;

    // adjust stride to account for padding
    dst_stride += dst_padding;

    // clear old data
    clear();

    if (Pixelformat::uid(src.fourcc) != Pixelformat::Enum::UNKNOWN) {
        // we know the format, so load the data right away
        m_width = src.width;
        m_height = src.height;
        m_pixfmt = pixfmt;
        Matrix<unsigned char>(src.buf, src.height, src.width * Pixelformat::bits(m_pixfmt) / 8,
                              dst_stride)
            .copy(m_buffer);
        return !m_buffer.empty();
    }

    // probe the target buffer size
    size_t dst_size = ImageConverter::probe(src, dst);
    if (dst_size == 0) {
        return false;
    }

    // resize target buffer if necessary
    if (dst_size > dst.buf_len) {
        m_buffer.resize(1, dst_size);
        dst.buf = m_buffer.data();
        dst.buf_len = m_buffer.rows() * m_buffer.step();
    }

    // looks like we need to convert the buffer
    bool converted = ImageConverter::Instance().convert(src, dst);
    if (!converted) {
        // something went wrong in the converter code
        clear();
        return false;
    }

    m_width = src.width;
    m_height = src.height;
    m_pixfmt = pixfmt;
    return true;
}

bool Image::convert(Pixelformat::Enum target) {
    size_t dst_padding;
    size_t dst_stride;
    size_t dst_pixel_size;
    ImageConverter::Source src = {};
    ImageConverter::Target dst = {};

    if (m_pixfmt == target) {
        return true;
    }

    src.buf = m_buffer.data();
    src.width = m_width;
    src.height = m_height;
    src.stride = m_buffer.step();
    src.fourcc = Pixelformat::fourcc(m_pixfmt);
    if (src.fourcc == 0) {
        return false;
    }

    dst.buf = m_buffer.data();
    dst.buf_len = m_buffer.rows() * m_buffer.step();
    dst.fourcc = Pixelformat::fourcc(target);
    if (dst.fourcc == 0) {
        return false;
    }

    dst_pixel_size = Pixelformat::bits(target) / 8;
    if (dst_pixel_size == 0) {
        return false;
    }

    // always align on word boundaries
    dst.alignment = 4;

    // calculate the target stride
    dst_stride = src.width * dst_pixel_size;

    // the general formula is:
    // padding = (ALIGN - (LENGTH % ALIGN)) % ALIGN
    dst_padding = (dst.alignment - (dst_stride % dst.alignment)) % dst.alignment;

    // adjust stride to account for padding
    dst_stride += dst_padding;

    // probe the target buffer size
    size_t dst_size = ImageConverter::probe(src, dst);
    if (dst_size == 0) {
        return false;
    }

    bool converted = false;
    std::vector<unsigned char> buf;

    // try to convert in-place first
    if (m_buffer.capacity() > 0) {
        converted = ImageConverter::Instance().convert(src, dst);
    }

    if (!converted) {
        // allocate a new buffer by first moving the contents to an alternative location
        Matrix<unsigned char> tmp;
        m_buffer.move(tmp);
        m_buffer.resize(src.height, src.width * dst_pixel_size, dst_stride);
        src.buf = tmp.data();
        dst.buf = m_buffer.data();
        dst.buf_len = m_buffer.rows() * m_buffer.step();
        converted = ImageConverter::Instance().convert(src, dst);
    }

    if (!converted) {
        // looks like we ran out of options
        clear();
        return false;
    }

    // at this point the new buffer is available
    m_pixfmt = target;
    return true;
}
