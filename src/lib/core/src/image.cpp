#include <seraphim/core/image.h>

using namespace sph::core;

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
