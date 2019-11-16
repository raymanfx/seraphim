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
    virtual const std::byte *data(size_t i = 0) const = 0;

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
     * @brief Image size in 2D.
     * @return Width, height tuple.
     */
    sph::Size2s size() const { return sph::Size2s(stride(), height()); }
};

/**
 * @brief Image reference implementation class.
 *
 * Buffered image with additional metadata.
 * The back buffer may consist of memory allocated by the instance itself or external data.
 */
class CoreImage : public Image {
public:
    CoreImage() = default;

    /**
     * @brief Empty image with no initial data.
     * @param width Width of the input data.
     * @param height Height of the input data.
     * @param pixfmt Pixelformat of the input data.
     */
    CoreImage(uint32_t width, uint32_t height, Pixelformat::Enum pixfmt);

    /**
     * @brief Image with buffered data.
     * @param data The raw data to use.
     * @param width Width of the input data.
     * @param height Height of the input data.
     * @param pixfmt Pixelformat of the input data.
     * @param stride Number of bytes per pixel row (default: auto).
     */
    CoreImage(std::byte *data, uint32_t width, uint32_t height, Pixelformat::Enum pixfmt,
              size_t stride = 0);

    /**
     * @brief Image with buffered data.
     * @param data The raw data to use.
     * @param width Width of the input data.
     * @param height Height of the input data.
     * @param pixfmt Pixelformat of the input data.
     * @param stride Number of bytes per pixel row (default: auto).
     */
    CoreImage(unsigned char *data, uint32_t width, uint32_t height, Pixelformat::Enum pixfmt,
              size_t stride = 0)
        : CoreImage(reinterpret_cast<std::byte *>(data), width, height, pixfmt, stride) {}

    const std::byte *data(size_t i = 0) const override { return m_buffer.data(i); }
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
     * @brief operator ==
     * @param rhs Right hand side.
     * @return True if equal, false otherwise.
     */
    bool operator==(const CoreImage &rhs) const { return m_buffer == rhs.m_buffer; }

    /**
     * @brief Get the pixel position in memory.
     * @param x X offset.
     * @param y Y offset.
     * @return Pixel address in memory.
     */
    unsigned char *pixel(uint32_t x, uint32_t y) {
        assert(x < m_width && y < m_height);
        return reinterpret_cast<unsigned char *>(m_buffer.data(y)) +
               x * Pixelformat::bits(m_pixfmt) / 8;
    }

    /**
     * @brief Subscript operator retrieving a single matrix element reference.
     *
     * Note that element indexing is used, i.e. the first element is at (1, 1).
     *
     * @param i Matrix row index.
     * @param j Matrix column index.
     * @return The Matrix element at the specified offsets.
     */
    unsigned char *operator()(uint32_t x, uint32_t y) {
        assert(x < m_width && y < m_height);
        return reinterpret_cast<unsigned char *>(m_buffer.data(y)) +
               x * Pixelformat::bits(m_pixfmt) / 8;
    }

    /**
     * @brief Subscript operator retrieving a single matrix element reference.
     *
     * Note that element indexing is used, i.e. the first element is at (1, 1).
     *
     * @param i Matrix row index.
     * @param j Matrix column index.
     * @return The Matrix element at the specified offsets.
     */
    unsigned char *operator()(uint32_t x, uint32_t y) const {
        assert(x < m_width && y < m_height);
        return reinterpret_cast<unsigned char *>(m_buffer.data(y)) +
               x * Pixelformat::bits(m_pixfmt) / 8;
    }

    class iterator {
    public:
        typedef iterator self_type;
        typedef unsigned char value_type;
        typedef unsigned char &reference;
        typedef unsigned char *pointer;
        typedef std::forward_iterator_tag iterator_category;
        iterator(CoreImage &img, uint32_t x, uint32_t y) : m_img(img), m_x(x), m_y(y) {}
        self_type operator++() {
            m_x++;
            if (m_x >= m_img.width()) {
                m_x = 0;
                m_y++;
            }
            assert(m_x <= m_img.width() && m_y <= m_img.height());
            return *this;
        }
        self_type operator++(int) {
            self_type i = *this;
            m_x++;
            if (m_x >= m_img.width()) {
                m_x = 0;
                m_y++;
            }
            assert(m_x <= m_img.width() && m_y <= m_img.height());
            return i;
        }
        value_type *operator*() { return m_img(m_x, m_y); }
        bool operator==(const self_type &rhs) { return m_img == rhs.m_img; }
        bool operator!=(const self_type &rhs) { return !(m_img == rhs.m_img); }

    private:
        CoreImage &m_img;
        uint32_t m_x;
        uint32_t m_y;
    };

    class const_iterator {
    public:
        typedef const_iterator self_type;
        typedef unsigned char value_type;
        typedef unsigned char &reference;
        typedef unsigned char *pointer;
        typedef std::forward_iterator_tag iterator_category;
        const_iterator(const CoreImage &img, uint32_t x, uint32_t y) : m_img(img), m_x(x), m_y(y) {}
        self_type operator++() {
            m_x++;
            if (m_x >= m_img.width()) {
                m_x = 0;
                m_y++;
            }
            assert(m_x <= m_img.width() && m_y <= m_img.height());
            return *this;
        }
        self_type operator++(int) {
            self_type i = *this;
            m_x++;
            if (m_x >= m_img.width()) {
                m_x = 0;
                m_y++;
            }
            assert(m_x <= m_img.width() && m_y <= m_img.height());
            return i;
        }
        const value_type *operator*() { return m_img(m_x, m_y); }
        bool operator==(const self_type &rhs) { return m_img == rhs.m_img; }
        bool operator!=(const self_type &rhs) { return !(m_img == rhs.m_img); }

    private:
        const CoreImage &m_img;
        uint32_t m_x;
        uint32_t m_y;
    };

    /**
     * @brief Begin of the image, points to its first pixel.
     * @return Forward iterator.
     */
    iterator begin() { return iterator(*this, 0, 0); }

    /**
     * @brief End of the image, points to its last pixel.
     * @return Forward iterator.
     */
    iterator end() { return iterator(*this, width() - 1, height() - 1); }

    /**
     * @brief Begin of the image, points to its first pixel.
     * @return Constant forward iterator.
     */
    const_iterator begin() const { return const_iterator(*this, 0, 0); }

    /**
     * @brief End of the image, points to its last pixel.
     * @return Constant forward iterator.
     */
    const_iterator end() const { return const_iterator(*this, width() - 1, height() - 1); }

private:
    /// matrix back buffer holding pixel data
    Matrix<std::byte> m_buffer;

    /// width in pixels
    uint32_t m_width = 0;

    /// height in pixels
    uint32_t m_height = 0;

    /// pixelformat
    Pixelformat::Enum m_pixfmt = Pixelformat::Enum::UNKNOWN;
};

} // namespace sph

#endif // SPH_CORE_IMAGE_H
