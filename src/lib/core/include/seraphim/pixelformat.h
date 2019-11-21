/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_PIXELFORMAT_H
#define SPH_CORE_PIXELFORMAT_H

#include <array>
#include <cstdint>

namespace sph {

static inline constexpr uint32_t fourcc(char a, char b, char c, char d) {
    return ((static_cast<uint32_t>(a) << 0) | (static_cast<uint32_t>(b) << 8) |
            (static_cast<uint32_t>(c) << 16) | (static_cast<uint32_t>(d) << 24));
}

/**
 * @brief Pixelformat of an image.
 * This is basically a listing of our internal types that we know and support.
 * That means we know the bits per pixel and other such properties.
 */
struct Pixelformat {
    /**
     * @brief Unique pixelformat identifiers.
     */
    enum class Enum {
        /* Grayscale formats */
        GRAY8,
        GRAY16,
        /* RGB formats */
        BGR24,
        BGR32,
        RGB24,
        RGB32,
        /* Luminance Chrominance formats */
    };

    /**
     * @brief Pixel color scheme.
     */
    enum class Color {
        /* Unspecified format */
        UNKNOWN,
        /* Defined formats */
        GRAY,
        BGR,
        RGB
    };

    /// Color scheme of the pixel.
    Color color = Color::UNKNOWN;

    /// Size of one pixel (all channels).
    size_t size = 0;

    Pixelformat() = default;

    /**
     * @brief Pixelformat
     * @param color Pixel color.
     * @param size Size of a full pixel (all channels).
     */
    constexpr Pixelformat(Color color, size_t size) : color(color), size(size) {}

    /**
     * @brief Pixelformat
     * @param fourcc Four character code.
     */
    constexpr Pixelformat(uint32_t fourcc) {
        switch (fourcc) {
        case sph::fourcc('G', 'R', 'E', 'Y'):
            color = Color::GRAY;
            size = 1;
            break;
        case sph::fourcc('Y', '1', '6', ' '):
            color = Color::GRAY;
            size = 2;
            break;
        case sph::fourcc('B', 'G', 'R', '3'):
            color = Color::BGR;
            size = 3;
            break;
        case sph::fourcc('B', 'G', 'R', '4'):
            color = Color::BGR;
            size = 4;
            break;
        case sph::fourcc('R', 'G', 'B', '3'):
            color = Color::RGB;
            size = 3;
            break;
        case sph::fourcc('R', 'G', 'B', '4'):
            color = Color::RGB;
            size = 4;
            break;
        }
    }

    /**
     * @brief Pixelformat
     * @param fmt Composite format description.
     */
    constexpr Pixelformat(Enum fmt) {
        switch (fmt) {
        case Enum::GRAY8:
            color = Color::GRAY;
            size = 1;
            break;
        case Enum::GRAY16:
            color = Color::GRAY;
            size = 2;
            break;
        case Enum::BGR24:
            color = Color::BGR;
            size = 3;
            break;
        case Enum::BGR32:
            color = Color::BGR;
            size = 4;
            break;
        case Enum::RGB24:
            color = Color::RGB;
            size = 3;
            break;
        case Enum::RGB32:
            color = Color::RGB;
            size = 4;
            break;
        }
    }

    /**
     * @brief Four character code.
     * @return Fourc character code integer representation.
     */
    constexpr uint32_t fourcc() const {
        switch (color) {
        case Color::GRAY:
            switch (size) {
            case 1:
                return sph::fourcc('G', 'R', 'E', 'Y');
            case 2:
                return sph::fourcc('Y', '1', '6', ' ');
            }
            break;
        case Color::BGR:
            switch (size) {
            case 3:
                return sph::fourcc('B', 'G', 'R', '3');
            case 4:
                return sph::fourcc('B', 'G', 'R', '4');
            }
            break;
        case Color::RGB:
            switch (size) {
            case 3:
                return sph::fourcc('R', 'G', 'B', '3');
            case 4:
                return sph::fourcc('R', 'G', 'B', '4');
            }
            break;
        default:
            return 0;
        }

        return 0;
    }

    /**
     * @brief Number of bits allocated for each pixel.
     * @return The amount of bits. Equals size * 8.
     */
    constexpr size_t depth() const { return size * 8; }

    /**
     * @brief Number of channels in the format.
     * @return The amount of channels.
     */
    constexpr uint32_t channels() const {
        switch (color) {
        case Color::GRAY:
            return 1;
        case Color::BGR:
        case Color::RGB:
            return 3;
        default:
            return 0;
        }
    }

    /**
     * @brief Check the validity of the format.
     * @return True if the format is valid, i.e. size > 0.
     */
    bool valid() const { return size > 0; }

    /**
     * @brief operator !
     * @return True if the format is valid, i.e. size > 0.
     */
    bool operator!() const { return !valid(); }
};

} // namespace sph

#endif // SPH_CORE_PIXELFORMAT_H
