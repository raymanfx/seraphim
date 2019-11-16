#include <seraphim/except.h>
#include <seraphim/image.h>

using namespace sph;

CoreImage::CoreImage(uint32_t width, uint32_t height, Pixelformat::Enum pixfmt)
    : m_width(width), m_height(height), m_pixfmt(pixfmt) {
    if (pixfmt == Pixelformat::Enum::UNKNOWN) {
        SPH_THROW(LogicException, "CoreImage must have a defined pixel format");
    }

    Matrix<std::byte>(m_height, m_width * Pixelformat::bits(m_pixfmt) / 8).copy(m_buffer);
}

CoreImage::CoreImage(std::byte *data, uint32_t width, uint32_t height, Pixelformat::Enum pixfmt,
                     size_t stride)
    : m_width(width), m_height(height), m_pixfmt(pixfmt) {
    if (stride == 0) {
        stride = m_width * Pixelformat::bits(m_pixfmt) / 8;
    }

    if (pixfmt == Pixelformat::Enum::UNKNOWN) {
        SPH_THROW(LogicException, "CoreImage must have a defined pixel format");
    }

    m_buffer = Matrix<std::byte>(data, m_height, m_width * Pixelformat::bits(m_pixfmt) / 8, stride);
}

void CoreImage::clear() {
    m_width = 0;
    m_height = 0;
    m_buffer.clear();
}
