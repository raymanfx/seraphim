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

/**
 * @brief Image interface.
 *
 * An image always has two dimensions for the time being.
 * If you need more dimensions, you should create a new class and describe the third dimension
 * in a meaningful way (e.g. time, pixel (voxel), etc).
 */
class Image {
public:
    virtual ~Image() = default;

    /**
     * @brief Retrieve a pointer to the underlying data of the image.
     * @param i The index of the image row.
     * @return Start of the first row of the image.
     */
    virtual const unsigned char *data(size_t i = 0) const = 0;

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
     * @return Depth as bits.
     */
    virtual uint32_t depth() const = 0;

    /**
     * @brief Image size in bytes.
     * @return Height * stride.
     */
    size_t size() const { return height() * stride(); }
};

/**
 * @brief Shallow image representation.
 *
 * Data is never copied. This class can be used to wrap external image data from arbitrary sources
 * to pass to interfaces expecting an sph::Image.
 */
class VolatileImage : public Image {
public:
    VolatileImage() = default;

    /**
     * @brief Image with external data.
     * @param data The raw data to use.
     * @param width Width of the input data.
     * @param height Height of the input data.
     * @param pixfmt Pixelformat of the input data.
     * @param stride Number of bytes per pixel row (default: auto).
     */
    VolatileImage(unsigned char *data, uint32_t width, uint32_t height, Pixelformat::Enum pixfmt,
                  size_t stride = 0);

    const unsigned char *data(size_t i = 0) const override { return m_data + i * m_stride; }
    bool empty() const override { return m_data == nullptr; }
    uint32_t width() const override { return m_width; }
    uint32_t height() const override { return m_height; }
    size_t stride() const override { return m_stride; }
    Pixelformat::Enum pixfmt() const override { return m_pixfmt; }
    uint32_t channels() const override { return Pixelformat::channels(m_pixfmt); }
    uint32_t depth() const override { return Pixelformat::bits(m_pixfmt); }

    /**
     * @brief Checks whether the instance is valid.
     * @return True if not empty and the pixelformat is known.
     */
    bool valid() const { return !empty() && m_pixfmt != Pixelformat::Enum::UNKNOWN; }

    bool operator!() const { return !valid(); }

private:
    /// pixel data
    const unsigned char *m_data = nullptr;

    /// width in pixels
    uint32_t m_width = 0;
    /// height in pixels
    uint32_t m_height = 0;
    /// pixelformat
    Pixelformat::Enum m_pixfmt = Pixelformat::Enum::UNKNOWN;
    /// amount of bytes per row
    size_t m_stride = 0;
};

/**
 * @brief Image reference implementation class.
 *
 * Buffered image with additional metadata.
 */
class BufferedImage : public Image {
public:
    BufferedImage() = default;

    /**
     * @brief Empty image with no initial data.
     * @param width Width of the input data.
     * @param height Height of the input data.
     * @param pixfmt Pixelformat of the input data.
     */
    BufferedImage(uint32_t width, uint32_t height, Pixelformat::Enum pixfmt);

    /**
     * @brief Image with buffered data.
     * @param data The raw data to copy to the instance buffer.
     * @param width Width of the input data.
     * @param height Height of the input data.
     * @param pixfmt Pixelformat of the input data.
     * @param stride Number of bytes per pixel row (default: auto).
     */
    BufferedImage(unsigned char *data, uint32_t width, uint32_t height, Pixelformat::Enum pixfmt,
                  size_t stride = 0);

    /**
     * @brief BufferedImage Create a buffered image from a shallow wrapper instance.
     * @param img Image acting as wrapper around raw data.
     */
    BufferedImage(const VolatileImage &img);

    const unsigned char *data(size_t i = 0) const override { return m_buffer.data(i); }
    bool empty() const override { return m_buffer.empty(); }
    uint32_t width() const override { return m_width; }
    uint32_t height() const override { return m_height; }
    size_t stride() const override { return m_buffer.step(); }
    Pixelformat::Enum pixfmt() const override { return m_pixfmt; }
    uint32_t channels() const override { return Pixelformat::channels(m_pixfmt); }
    uint32_t depth() const override { return Pixelformat::bits(m_pixfmt); }

    /**
     * @brief Clear the internal buffer contents.
     */
    void clear();

    /**
     * @brief Resize the internal buffer.
     * @param width Target width.
     * @param height Target height.
     */
    void resize(uint32_t width, uint32_t height) {
        m_buffer.resize(height, stride());
        m_width = width;
        m_height = height;
    }

    /**
     * @brief Checks whether the instance is valid.
     * @return True if not empty and the pixelformat is known.
     */
    bool valid() const { return !empty() && m_pixfmt != Pixelformat::Enum::UNKNOWN; }

    bool operator!() const { return !valid(); }

    /**
     * @brief Get the pixel position in memory.
     * @param x X offset.
     * @param y Y offset.
     * @return Pixel address in memory.
     */
    unsigned char *pixel(uint32_t x, uint32_t y) {
        assert(x < m_width && y < m_height);
        return m_buffer.data(y) + x * Pixelformat::bits(m_pixfmt) / 8;
    }

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

} // namespace sph

#endif // SPH_CORE_IMAGE_H
