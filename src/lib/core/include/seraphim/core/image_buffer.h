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
    ImageBuffer();
    ~ImageBuffer();

    /**
     * @brief Pixelformat of an image buffer.
     * This is basically a listing of our internal types that we know and support.
     * That means we know the bits per pixel and other such properties.
     */
    enum class Pixelformat {
        /* RGB formats */
        BGR24,
        BGR32,
        RGB24,
        RGB32,
        /* Luminance Chrominance formats */
        Y16,
        /* Unspecified format */
        UNKNOWN
    };

    /**
     * @brief Convert an arbitrary four character code to a known pixelformat.
     * If none of the internal formats can be matched, FMT_UNKNOWN is used.
     * @param fourcc The four character code.
     * @return A known format or FMT_CUSTOM if we cannot handle it.
     */
    static Pixelformat as_pixelformat(const uint32_t &fourcc);

    /**
     * @brief Image format description.
     */
    struct Format {
        /// width in pixels
        uint32_t width;
        /// height in pixels
        uint32_t height;
        /// pixel row padding
        uint32_t padding = 0;
        /// pixelformat
        Pixelformat pixfmt = Pixelformat::UNKNOWN;
    };

    /**
     * @brief Check whether the buffer is empty.
     * @return True if no buffer is set or its size is zero.
     */
    bool empty() const { return (m_data == nullptr) || (m_size == 0); }

    /**
     * @brief Yields information on whether or not the instance holds allocated pixel data.
     * @return True if buffer is managed by the instance, false otherwise.
     */
    bool owns_data() const { return m_data_owned; }

    /**
     * @brief Clear the buffer, freeing any allocated resources.
     */
    void reset();

    /**
     * @brief Copy an image buffer so this instance owns the data.
     * @param src Address of the source buffer.
     * @param fmt Pixelformat of the source buffer.
     * @return True on success, false otherwise.
     */
    bool load(unsigned char *src, const Format &fmt);

    /**
     * @brief Copy an image buffer so this instance owns the data.
     *        Convert the buffer if necessary prior to loading it.
     * @param src Address of the source buffer.
     * @param src_fmt Pixelformat of the source buffer.
     * @param pixfmt Pixelformat of the target buffer.
     * @return True on success, false otherwise.
     */
    bool load(unsigned char *src, const ImageBufferConverter::SourceFormat &src_fmt,
              const Pixelformat &pixfmt);

    /**
     * @brief Wrap an image buffer and optionally pass ownership to the instance.
     * @param src Address of the source buffer.
     * @param fmt Pixelformat of the source buffer.
     * @param ownership Whether the image instance shall own the data (and free it later).
     * @return True on success, false otherwise.
     */
    bool assign(unsigned char *src, const Format &fmt, const bool &ownership = false);

    /**
     * @brief Pointer to the internal pixel buffer.
     * @return Pixel buffer as bytes.
     */
    const unsigned char *data() const { return m_data; }

    /**
     * @brief Internal pixel buffer size.
     * @return Pixel buffer size in bytes.
     */
    size_t size() const { return m_size; }

    /**
     * @brief Buffer format description.
     * @return @ref Format struct.
     */
    Format format() const { return m_format; }

    /**
     * @brief Get the scanline position in memory.
     * @param y Y offset.
     * @return Pixel scanline address in memory.
     */
    unsigned char *scanline(const uint32_t &y) const;

    /**
     * @brief Get the pixel position in memory.
     * @param x X offset.
     * @param y Y offset.
     * @return Pixel address in memory.
     */
    unsigned char *pixel(const uint32_t &x, const uint32_t &y) const;

    /**
     * @brief Convert between internal formats.
     * @param target Target pixel format.
     * @return True on success, false otherwise.
     */
    bool convert(const Pixelformat &target);

protected:
    /// pixel buffer address, must reside in host memory (DRAM) for now
    unsigned char *m_data;
    /// size of the pixel buffer in bytes
    size_t m_size;
    /// whether the instance owns the buffer
    bool m_data_owned;

    /// buffer format
    Format m_format;
};

} // namespace core
} // namespace sph

#endif // SPH_CORE_IMAGE_BUFFER_H
