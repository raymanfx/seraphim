#include <seraphim/except.h>
#include <seraphim/image.h>

using namespace sph;

CoreImage::CoreImage(uint32_t width, uint32_t height, const Pixelformat &pixfmt)
    : m_width(width), m_height(height), m_pixfmt(pixfmt) {
    Matrix<std::byte>(m_height, m_width * m_pixfmt.size).copy(m_buffer);
}

CoreImage::CoreImage(std::byte *data, uint32_t width, uint32_t height, const Pixelformat &pixfmt,
                     size_t stride)
    : m_width(width), m_height(height), m_pixfmt(pixfmt) {
    if (stride == 0) {
        stride = m_width * m_pixfmt.size;
    }

    m_buffer = Matrix<std::byte>(data, m_height, m_width * m_pixfmt.size, stride);
}

void CoreImage::clear() {
    m_width = 0;
    m_height = 0;
    m_buffer.clear();
}
