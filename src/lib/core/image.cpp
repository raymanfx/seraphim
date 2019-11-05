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

BufferedImage::BufferedImage(uint32_t width, uint32_t height, Pixelformat::Enum pixfmt)
    : m_width(width), m_height(height), m_pixfmt(pixfmt) {
    if (pixfmt == Pixelformat::Enum::UNKNOWN) {
        SPH_THROW(LogicException, "BufferedImage must have a defined pixel format");
    }

    Matrix<unsigned char>(m_height, m_width * Pixelformat::bits(m_pixfmt) / 8).copy(m_buffer);
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
