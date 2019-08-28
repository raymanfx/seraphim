/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_IMAGE_BUFFER_H
#define SPH_CORE_IMAGE_BUFFER_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "image_buffer_converter.h"

namespace sph {
namespace core {

/**
 * @brief Image buffer class, represents an object consisting of uncompressed pixels.
 * The pixelbuffer format is represented by a four character code similar
 * to Linux v4l2: https://linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/pixfmt.html.
 */
class ImageBuffer {
public:
    ImageBuffer() = default;
    ImageBuffer(const ImageBuffer &buf);
    ~ImageBuffer();

    /**
     * @brief Assignment operator, performing a shallow copy.
     * @param buf Instance to copy from.
     */
    void operator=(const ImageBuffer &buf);

    /**
     * @brief Pixelformat of an image buffer.
     * This is basically a listing of our internal types that we know and support.
     * That means we know the bits per pixel and other such properties.
     */
    enum class Pixelformat {
        /* Unspecified format */
        UNKNOWN,
        /* RGB formats */
        BGR24 = fourcc('B', 'G', 'R', '3'),
        BGR32 = fourcc('B', 'G', 'R', '4'),
        RGB24 = fourcc('R', 'G', 'B', '3'),
        RGB32 = fourcc('R', 'G', 'B', '4'),
        /* Luminance Chrominance formats */
        Y8 = fourcc('G', 'R', 'E', 'Y'),
        Y16 = fourcc('Y', '1', '6', ' ')
    };

    /**
     * @brief Convert an arbitrary four character code to a known pixelformat.
     * If none of the internal formats can be matched, FMT_UNKNOWN is used.
     * @param fourcc The four character code.
     * @return A known format or FMT_CUSTOM if we cannot handle it.
     */
    static Pixelformat fourcc2pixfmt(const uint32_t &fourcc);

    /**
     * @brief Convert a well defined pixelformat to its fourcc representation.
     * @param fmt The pixelformat.
     * @return Four character code.
     */
    static uint32_t pixfmt2fourcc(const Pixelformat &fmt) { return static_cast<uint32_t>(fmt); }

    /**
     * @brief Image format description.
     */
    struct Format {
        /// width in pixels
        uint32_t width;
        /// height in pixels
        uint32_t height;
        /// pixelformat
        Pixelformat pixfmt = Pixelformat::UNKNOWN;
        /// length of one pixel row in bytes
        /// if unset (0), this will be initialized as follows:
        ///   stride = width * bytes(pixfmt)
        size_t stride = 0;
    };

    /**
     * @brief Check whether the buffer is empty.
     * @return True if no buffer is set or its size is zero.
     */
    bool empty() const { return (m_data == nullptr) || (size() == 0); }

    /**
     * @brief Clear the buffer, freeing any allocated resources.
     *        Does not perform any reallocations. Use @ref shrink if you want to reclaim memory.
     */
    void clear();

    /**
     * @brief Reclaim unneeded memory that is occupied by the internal buffer.
     *        Since this will cause reallocation, use it only if you absolutely need to reclaim
     *        the occupied memory.
     */
    void shrink();

    /**
     * @brief Copy an image buffer so this instance owns the data.
     * @param buf The source image buffer to load data from.
     * @return True on success, false otherwise.
     */
    bool load(const ImageBuffer &buf);

    /**
     * @brief Copy an image buffer so this instance owns the data.
     * @param src Address of the source buffer.
     * @param fmt Pixelformat of the source buffer.
     * @return True on success, false otherwise.
     */
    bool load(const unsigned char *src, const Format &fmt);

    /**
     * @brief Copy an image buffer so this instance owns the data.
     *        Convert the buffer if necessary prior to loading it.
     * @param src Address of the source buffer.
     * @param src_fmt Pixelformat of the source buffer.
     * @param pixfmt Pixelformat of the target buffer.
     * @return True on success, false otherwise.
     */
    bool load(const unsigned char *src, const ImageBufferConverter::SourceFormat &src_fmt,
              const Pixelformat &pixfmt);

    /**
     * @brief Wrap an image buffer and optionally pass ownership to the instance.
     * @param src Address of the source buffer.
     * @param fmt Pixelformat of the source buffer.
     * @return True on success, false otherwise.
     */
    bool assign(unsigned char *src, const Format &fmt);

    /**
     * @brief Pointer to the internal pixel buffer.
     * @return Pixel buffer as bytes.
     */
    const unsigned char *data() const { return m_data; }

    /**
     * @brief Buffer format description.
     * @return @ref Format struct.
     */
    Format format() const { return m_format; }

    /**
     * @brief Pixel row alignment, determined by padding bytes.
     *        Reasonable values are:
     *          1 (one-byte alignment)
     *          4 (word alignment, default)
     * @return Alignment in bytes.
     */
    uint8_t row_alignment() const { return m_format.stride & 3 ? 1 : 4; }

    /**
     * @brief Calculate the size of the underlying buffer (including padding).
     * @return Size in bytes.
     */
    inline size_t size() const { return m_format.height * m_format.stride; }

    /**
     * @brief Get the scanline position in memory.
     * @param y Y offset.
     * @return Pixel scanline address in memory.
     */
    const unsigned char *scanline(const uint32_t &y) const;

    /**
     * @brief Get the pixel position in memory.
     * @param x X offset.
     * @param y Y offset.
     * @return Pixel address in memory.
     */
    const unsigned char *pixel(const uint32_t &x, const uint32_t &y) const;

    /**
     * @brief Convert between internal formats.
     * @param target Target pixel format.
     * @return True on success, false otherwise.
     */
    bool convert(const Pixelformat &target);

protected:
    /// pixel buffer address, must reside in host memory (DRAM) for now
    const unsigned char *m_data = nullptr;
    /// back buffer, used for internal data allocations
    std::vector<unsigned char> m_data_buffer;

    /// buffer format
    Format m_format = {};
};

} // namespace core
} // namespace sph

#endif // SPH_CORE_IMAGE_BUFFER_H
