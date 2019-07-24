#include <seraphim/core/image.h>

using namespace sph::core;

Image::Image(const size_t &width, const size_t &height, const int &channels) {
    m_metadata.dimensions.push_back(width);
    m_metadata.dimensions.push_back(height);
    m_metadata.channels = channels;
}

Image::~Image() {
    // dummy
}

bool Image::empty() const {
    if (m_metadata.dimensions.size() > 0) {
        return false;
    } else if (m_metadata.dimensions.size() == 0) {
        return true;
    }

    for (auto &dim : m_metadata.dimensions) {
        if (dim > 0) {
            return false;
        }
    }

    return ImageBuffer::empty();
}

bool Image::valid() const {
    if (m_data == nullptr || m_data_size == 0) {
        return false;
    }

    if (empty()) {
        return false;
    }

    return true;
}
