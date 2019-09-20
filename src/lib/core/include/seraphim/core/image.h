/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_IMAGE_H
#define SPH_CORE_IMAGE_H

#include <cstddef>
#include <cstdint>
#include <map>

#include "image_converter.h"
#include "matrix.h"
#include "pixelformat.h"

namespace sph {
namespace core {

/**
 * @brief Image interface.
 *
 * An image always has two dimensions for the time being.
 * If you need more dimensions, you should create a new class and describe the third dimension
 * in a meaningful way (e.g. time, pixel (voxel), etc).
 */
class IImage {
public:
    virtual ~IImage() = default;

    /**
     * @brief Retrieve a pointer to the underlying data of the image.
     * @return Start of the first row of the image.
     */
    virtual const unsigned char *data() const = 0;

    /**
     * @brief Check whether the buffer is empty.
     * @return True if no buffer is set.
     */
    virtual bool empty() const = 0;

    /**
     * @brief Width of the image.
     * @return Width in pixels.
     */
    virtual uint32_t width() const = 0;

    /**
     * @brief Height of the image.
     * @return Height in pixels.
     */
    virtual uint32_t height() const = 0;

    /**
     * @brief Length of one pixel row (including padding).
     *        For images with no padding, this equals height * sizeof(pixel).
     * @return Length in bytes.
     */
    virtual size_t stride() const = 0;

    /**
     * @brief Format of each pixel in the image.
     * @return Pixelformat UID, see @ref Pixelformat.
     */
    virtual Pixelformat::Enum pixfmt() const = 0;

    /**
     * @brief The number of channels of the image.
     * @return 1 for grayscale, 3 for BGR, 4 for BGRA.
     */
    virtual uint32_t channels() const = 0;

    /**
     * @brief The number of bits for each channel.
     *        E.g. for RGB32 this would return 32.
     * @return
     */
    virtual uint32_t depth() const = 0;

    /**
     * @brief Unary "not" operator, checks whether the instance is valid.
     * @return True if the image is not empty, false otherwise.
     */
    virtual bool operator!() const = 0;
};

/**
 * @brief Image reference implementation class, buffered image with additional metadata.
 *
 * The buffer can either load (hold) data which means it owns it or assign data, which means
 * the data was gathered from any memory location and may be used, but not altered.
 */
class Image : public IImage {
public:
    Image() = default;
    Image(unsigned char *data, const uint32_t &width, const uint32_t &height,
          const Pixelformat::Enum &pixfmt, const size_t &stride = 0);

    const unsigned char *data() const override { return m_buffer.data(); }
    bool empty() const override { return m_buffer.empty(); }
    uint32_t width() const override { return m_width; }
    uint32_t height() const override { return m_height; }
    size_t stride() const override { return m_buffer.step(); }
    Pixelformat::Enum pixfmt() const override { return m_pixfmt; }
    uint32_t channels() const override;
    uint32_t depth() const override;
    bool operator!() const override { return !validate(); }

    void clear();
    bool validate() const { return !empty() && m_pixfmt != Pixelformat::Enum::UNKNOWN; }

    /**
     * @brief Get the scanline position in memory.
     * @param y Y offset.
     * @return Pixel scanline address in memory.
     */
    unsigned char *scanline(const uint32_t &y) const {
        assert(y < m_height);
        return m_buffer[y];
    }

    /**
     * @brief Get the pixel position in memory.
     * @param x X offset.
     * @param y Y offset.
     * @return Pixel address in memory.
     */
    unsigned char *pixel(const uint32_t &x, const uint32_t &y) const {
        assert(x < m_width && y < m_height);
        if (m_pixfmt == Pixelformat::Enum::UNKNOWN) {
            return nullptr;
        }
        return m_buffer[y] + x * Pixelformat::bits(m_pixfmt) / 8;
    }

    /**
     * @brief Copy an image buffer so this instance owns the data.
     *        Convert the buffer if necessary prior to loading it.
     * @param src Address of the source buffer.
     * @param pixfmt Pixelformat of the target buffer.
     * @return True on success, false otherwise.
     */
    bool load(const ImageConverter::Source &src, const Pixelformat::Enum &pixfmt);

    /**
     * @brief Convert between internal formats.
     * @param target Target pixel format.
     * @return True on success, false otherwise.
     */
    bool convert(const Pixelformat::Enum &target);

private:
    /// matrix back buffer holding pixel data
    Matrix<unsigned char> m_buffer;

    /// width in pixels
    uint32_t m_width = 0;
    /// height in pixels
    uint32_t m_height = 0;
    /// pixelformat
    Pixelformat::Enum m_pixfmt = Pixelformat::Enum::UNKNOWN;
};

} // namespace core
} // namespace sph

#endif // SPH_CORE_IMAGE_H
