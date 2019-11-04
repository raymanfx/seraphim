#include <seraphim/except.h>
#include <seraphim/image.h>

using namespace sph;

VolatileImage::VolatileImage(unsigned char *data, uint32_t width, uint32_t height,
                             Pixelformat::Enum pixfmt, size_t stride)
    : m_data(data), m_width(width), m_height(height), m_pixfmt(pixfmt), m_stride(stride) {
    if (m_stride == 0) {
        m_stride = m_width * Pixelformat::bits(m_pixfmt) / 8;
    }
}

BufferedImage::BufferedImage(unsigned char *data, uint32_t width, uint32_t height,
                             Pixelformat::Enum pixfmt, size_t stride)
    : m_width(width), m_height(height), m_pixfmt(pixfmt) {
    if (stride == 0) {
        stride = m_width * Pixelformat::bits(m_pixfmt) / 8;
    }

    if (pixfmt == Pixelformat::Enum::UNKNOWN) {
        SPH_THROW(LogicException, "BufferedImage must have a defined pixel format");
    }

    Matrix<unsigned char>(data, m_height, m_width * Pixelformat::bits(m_pixfmt) / 8, stride)
        .copy(m_buffer);
}

BufferedImage::BufferedImage(const VolatileImage &img)
    : m_width(img.width()), m_height(img.height()), m_pixfmt(img.pixfmt()) {
    Matrix<unsigned char>(const_cast<unsigned char *>(img.data()), m_height,
                          m_width * Pixelformat::bits(m_pixfmt) / 8, img.stride())
        .copy(m_buffer);
}

void BufferedImage::clear() {
    m_width = 0;
    m_height = 0;
    m_buffer.clear();
}

bool BufferedImage::convert(Pixelformat::Enum target) {
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
        m_buffer.resize(src.height, dst_stride);
        src.buf = tmp.data();
        dst.buf = m_buffer.data();
        dst.buf_len = m_buffer.rows() * m_buffer.step();
        converted = ImageConverter::Instance().convert(src, dst);
    }

    if (!converted) {
        // looks like we ran out of options
        m_buffer.clear();
        return false;
    }

    // at this point the new buffer is available
    m_pixfmt = target;
    return true;
}
