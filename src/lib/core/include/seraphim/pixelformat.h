/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_PIXELFORMAT_H
#define SPH_CORE_PIXELFORMAT_H

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
        /* Unspecified format */
        UNKNOWN,
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
     * @brief @ref Pixelformat UID for a given four character code.
     * @param fourcc Fourc character code integer representationr.
     * @return The @ref Pixelformat identifier.
     */
    static Pixelformat::Enum uid(uint32_t fourcc) {
        switch (fourcc) {
        case sph::fourcc('G', 'R', 'E', 'Y'):
            return Pixelformat::Enum::GRAY8;
        case sph::fourcc('Y', '1', '6', ' '):
            return Pixelformat::Enum::GRAY16;
        case sph::fourcc('B', 'G', 'R', '3'):
            return Pixelformat::Enum::BGR24;
        case sph::fourcc('B', 'G', 'R', '4'):
            return Pixelformat::Enum::BGR32;
        case sph::fourcc('R', 'G', 'B', '3'):
            return Pixelformat::Enum::RGB24;
        case sph::fourcc('R', 'G', 'B', '4'):
            return Pixelformat::Enum::RGB32;
        default:
            return Pixelformat::Enum::UNKNOWN;
        }
    }

    /**
     * @brief Four character codes for a given @ref Pixelformat UID.
     * @param uid The @ref Pixelformat identifier.
     * @return Fourc character code integer representation.
     */
    static uint32_t fourcc(Pixelformat::Enum uid) {
        switch (uid) {
        case Pixelformat::Enum::GRAY8:
            return sph::fourcc('G', 'R', 'E', 'Y');
        case Pixelformat::Enum::GRAY16:
            return sph::fourcc('Y', '1', '6', ' ');
        case Pixelformat::Enum::BGR24:
            return sph::fourcc('B', 'G', 'R', '3');
        case Pixelformat::Enum::BGR32:
            return sph::fourcc('B', 'G', 'R', '4');
        case Pixelformat::Enum::RGB24:
            return sph::fourcc('R', 'G', 'B', '3');
        case Pixelformat::Enum::RGB32:
            return sph::fourcc('R', 'G', 'B', '4');
        default:
            return 0;
        }
    }

    /**
     * @brief Number of bits allocated for each pixel.
     * @param uid The @ref Pixelformat identifier.
     * @return The amount of bits. Usually a multiple of eight.
     */
    static uint32_t bits(Pixelformat::Enum uid) {
        switch (uid) {
        case Pixelformat::Enum::GRAY8:
            return 8;
        case Pixelformat::Enum::GRAY16:
            return 16;
        case Pixelformat::Enum::BGR24:
            return 24;
        case Pixelformat::Enum::BGR32:
            return 32;
        case Pixelformat::Enum::RGB24:
            return 24;
        case Pixelformat::Enum::RGB32:
            return 32;
        default:
            return 0;
        }
    }

    /**
     * @brief Number of channels in the format.
     * @param uid The @ref Pixelformat identifier.
     * @return The amount of channels.
     */
    static uint32_t channels(Pixelformat::Enum uid) {
        switch (uid) {
        case Pixelformat::Enum::GRAY8:
        case Pixelformat::Enum::GRAY16:
            return 1;
        case Pixelformat::Enum::BGR24:
        case Pixelformat::Enum::BGR32:
        case Pixelformat::Enum::RGB24:
        case Pixelformat::Enum::RGB32:
            return 3;
        default:
            return 0;
        }
    }
};

} // namespace sph

#endif // SPH_CORE_PIXELFORMAT_H
