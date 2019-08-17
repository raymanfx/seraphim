/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_IMAGE_H
#define SPH_CORE_IMAGE_H

#include "image_buffer.h"

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
     * @brief Check whether the buffer is empty.
     * @return True if no buffer is set.
     */
    virtual bool empty() const = 0;

    /**
     * @brief Width of the image.
     * @return Width in pixels.
     */
    virtual size_t width() const = 0;

    /**
     * @brief Height of the image.
     * @return Height in pixels.
     */
    virtual size_t height() const = 0;

    /**
     * @brief The number of bits for each channel.
     *        E.g. for RGB32 this would return 32.
     * @return
     */
    virtual int depth() const = 0;

    /**
     * @brief The number of channels of the image.
     * @return 1 for grayscale, 3 for BGR, 4 for BGRA.
     */
    virtual int channels() const = 0;

    /**
     * @brief RGB Pixel data.
     */
    struct RGBPixel {
        /// red [0, 255]
        int r;
        /// green [0, 255]
        int g;
        /// blue [0, 255]
        int b;
        /// transparency, 0 is opaque, 255 is transparent
        int a;
    };

    /**
     * @brief Read RGB pixel intensities at the specified offsets.
     * @param x X offset.
     * @param y Y offset.
     * @return Pixel intensities.
     */
    virtual struct RGBPixel rgb(const uint32_t &x, const uint32_t &y) const = 0;

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
    explicit Image(const ImageBuffer &buf);

    bool empty() const override { return m_buffer.empty(); }
    size_t width() const override { return m_buffer.format().width; }
    size_t height() const override { return m_buffer.format().height; }
    int depth() const override;
    int channels() const override;
    struct RGBPixel rgb(const uint32_t &x, const uint32_t &y) const override;
    bool operator!() const override { return !empty(); }

    /**
     * @brief Const reference to the underlying pixel buffer.
     * @return The pixel buffer instance.
     */
    const ImageBuffer &buffer() const { return m_buffer; }

    /**
     * @brief Mutable reference to the underlying pixel buffer.
     * @return The pixel buffer instance.
     */
    ImageBuffer &mutable_buffer() { return m_buffer; }

private:
    /// buffer to hold the underlying pixel data
    ImageBuffer m_buffer;
};

} // namespace core
} // namespace sph

#endif // SPH_CORE_IMAGE_H
