#include <seraphim/core/image.h>

using namespace sph::core;

Image::Image(const ImageBuffer &buf) {
    m_buffer = buf;
}

int Image::depth() const {
    switch (m_buffer.format().pixfmt) {
    case ImageBuffer::Pixelformat::BGR24:
    case ImageBuffer::Pixelformat::BGR32:
    case ImageBuffer::Pixelformat::RGB24:
    case ImageBuffer::Pixelformat::RGB32:
        return 24;
    case ImageBuffer::Pixelformat::Y16:
        return 16;
    default:
        return 0;
    }
}

int Image::channels() const {
    switch (m_buffer.format().pixfmt) {
    case ImageBuffer::Pixelformat::BGR24:
    case ImageBuffer::Pixelformat::BGR32:
    case ImageBuffer::Pixelformat::RGB24:
    case ImageBuffer::Pixelformat::RGB32:
        return 3;
    case ImageBuffer::Pixelformat::Y16:
        return 1;
    default:
        return 0;
    }
}

struct Image::RGBPixel Image::rgb(const uint32_t &x, const uint32_t &y) const {
    struct RGBPixel pixel = {};
    const unsigned char *pixel_addr = m_buffer.pixel(x, y);

    if (pixel_addr == nullptr) {
        // something went wrong
        return pixel;
    }

    switch (m_buffer.format().pixfmt) {
    case ImageBuffer::Pixelformat::BGR24:
    case ImageBuffer::Pixelformat::BGR32:
        pixel.r = pixel_addr[2];
        pixel.g = pixel_addr[1];
        pixel.b = pixel_addr[0];
        pixel.a = 0;
        break;
    case ImageBuffer::Pixelformat::RGB24:
    case ImageBuffer::Pixelformat::RGB32:
        pixel.r = pixel_addr[0];
        pixel.g = pixel_addr[1];
        pixel.b = pixel_addr[2];
        pixel.a = 0;
        break;
    case ImageBuffer::Pixelformat::Y16:
        pixel.r = pixel_addr[0];
        pixel.g = pixel_addr[0];
        pixel.b = pixel_addr[0];
        pixel.a = 0;
        break;
    default:
        break;
    }

    return pixel;
}
