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

namespace sph {
namespace core {

static inline constexpr uint32_t fourcc(const char &a, const char &b, const char &c,
                                        const char &d) {
    return ((static_cast<uint32_t>(a) << 0) | (static_cast<uint32_t>(b) << 8) |
            (static_cast<uint32_t>(c) << 16) | (static_cast<uint32_t>(d) << 24));
}

static inline std::string fourcc_str(uint32_t fourcc) {
    char str[4];
    std::strncpy(str, reinterpret_cast<char *>(&fourcc), 4);
    return std::string(str);
}

/**
 * @brief Image buffer class, represents an object consisting of pixels.
 * The pixelbuffer format is represented by a four character code similar
 * to Linux v4l2: https://linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/pixfmt.html.
 */
class ImageBuffer {
public:
    ImageBuffer();
    ~ImageBuffer();

    enum class Pixelformat {
        /* RGB formats */
        FMT_RGB24 = fourcc('R', 'G', 'B', '3'),
        /* Luminance Chrominance formats */
        FMT_YUYV = fourcc('Y', 'U', 'Y', 'V'),
        /* Compressed formats */
        FMT_MJPG = fourcc('M', 'J', 'P', 'G'),
        /* Unspecified format */
        FMT_CUSTOM
    };

    /**
     * @brief Convert an arbitrary four character code to a known pixelformat.
     * If none of the internal formats can be matched, FMT_CUSTOM is used.
     * @param fourcc The four character code.
     * @return A known format or FMT_CUSTOM if we cannot handle it.
     */
    static Pixelformat as_pixelformat(const uint32_t &fourcc);

    /**
     * @brief Copy an image buffer so this instance owns the data.
     * @param src Address of the source buffer.
     * @param len Length of the source buffer.
     * @param fmt Pixelformat of the source buffer.
     * @return True on success, false otherwise.
     */
    bool copy_data(void *src, const size_t &len, const Pixelformat &fmt);

    /**
     * @brief Wrap an image buffer and optionally pass ownership to the instance.
     * @param src Address of the source buffer.
     * @param len Length of the source buffer.
     * @param fmt Pixelformat of the source buffer.
     * @param ownership Whether the image instance shall own the data (and free it later).
     * @return True on success, false otherwise.
     */
    bool wrap_data(void *src, const size_t &len, const Pixelformat &fmt,
                   const bool &ownership = false);

    /**
     * @brief Pointer to the internal pixel buffer.
     * @return Pixel buffer as bytes.
     */
    const void *data() const { return m_data; }

    /**
     * @brief Internal pixel buffer size.
     * @return Pixel buffer size in bytes.
     */
    size_t data_size() const { return m_data_size; }

    /**
     * @brief Buffer pixelformat description.
     * @return @ref Pixelformat.
     */
    Pixelformat pixelformat() const { return m_pixelformat; }

    /**
     * @brief Check whether the buffer is empty.
     * @return True if no buffer is set or its size is zero.
     */
    bool empty() const { return (m_data == nullptr) || (m_data_size == 0); }

    /**
     * @brief Yields information on whether or not the instance holds allocated pixel data.
     * @return True if buffer is managed by the instance, false otherwise.
     */
    bool owns_data() const { return m_data_owned; }

protected:
    /// pixel buffer address, must reside in host memory (DRAM) for now
    void *m_data;
    /// size of the pixel buffer in bytes
    size_t m_data_size;
    /// whether the instance owns the buffer
    bool m_data_owned;

    /// buffer pixel format
    Pixelformat m_pixelformat;
};

} // namespace core
} // namespace sph

#endif // SPH_CORE_IMAGE_BUFFER_H
