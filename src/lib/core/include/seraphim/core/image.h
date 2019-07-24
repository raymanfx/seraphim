/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_IMAGE_H
#define SPH_CORE_IMAGE_H

#include <vector>

#include "image_buffer.h"

namespace sph {
namespace core {

/**
 * @brief Image class, represents an image buffer with additional metadata.
 *
 * An image can have two dimensions (flat camera image), three dimensions (stereo camera image) or
 * any arbitrary number of dimensions really, as long as their meaning is defined.
 * For now, only 2D images are supported.
 */
class Image : public ImageBuffer {
public:
    Image(const size_t &width, const size_t &height, const int &channels);
    ~Image();

    struct Metadata {
        /// image channels, e.g. 1 for a grayscale image
        int channels;
        /// image dimensions, [0] -> x-axis, [1] -> y-axis, etc
        std::vector<size_t> dimensions;
    };

    /**
     * @brief Convenience API, checks whether all dimensions have size zero.
     * @return True if the image has no size, false otherwise.
     */
    bool empty() const;

    /**
     * @brief Convenience API, checks whether the image is valid.
     * @return True if the image has a backing buffer, valid dimensions, false otherwise.
     */
    bool valid() const;

    /**
     * @brief Convenience API, returns the size of the first dimension (x axis).
     * @return Size in pixels.
     */
    size_t width() const { return m_metadata.dimensions.size() > 0 ? m_metadata.dimensions[0] : 0; }

    /**
     * @brief Convenience API, returns the size of the second dimension (y axis).
     * @return Size in pixels.
     */
    size_t height() const {
        return m_metadata.dimensions.size() > 1 ? m_metadata.dimensions[1] : 0;
    }

private:
    struct Metadata m_metadata;
};

} // namespace core
} // namespace sph

#endif // SPH_CORE_IMAGE_H
