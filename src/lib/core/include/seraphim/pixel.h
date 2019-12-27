/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_PIXEL_H
#define SPH_CORE_PIXEL_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <ostream>

#include "pixelformat.h"

namespace sph {

/**
 * Abstract pixel structure.
 */
template <typename T = uint8_t, size_t N = sizeof(T)> struct Pixel {
public:
    Pixel() {}

    /// raw data store for intensity values
    std::array<T, N> data;

    /// size of the pixel (min. the sum of its components)
    constexpr size_t size() const { return N; }
};

/**
 * @brief Ostream operator.
 * @param os Source ostream.
 * @param pix Pixel instance.
 * @return Modified ostream.
 */
template <typename T, size_t N> std::ostream &operator<<(std::ostream &os, const Pixel<T, N> &pix) {
    os << "[";
    for (size_t i = 0; i < pix.data.size(); i++) {
        if (i > 0) {
            os << " ";
        }
        os << pix.data[i];
    }
    os << "]";

    return os;
}

namespace pix {

/**
 * @brief A single gray pixel.
 * Consists of a single Y component of defined size.
 */
template <typename T = uint8_t, size_t N = sizeof(T)> class MONO : public Pixel<T, N> {
    static_assert(N >= 1, "pixel size must be >= 1");

public:
    MONO() {}
    MONO(T y) : Pixel<T, N>::data(y) {}

    T &y = Pixel<T, N>::data[0];
};

/**
 * @brief A single RGB pixel.
 * Consists of three components of defined size, R, G and B.
 */
template <typename T = uint8_t, size_t N = 3> class RGB : public Pixel<T, N> {
    static_assert(N >= 3, "pixel size must be >= 3");

public:
    RGB() {}
    RGB(T r, T g, T b) {
        this->r() = r;
        this->g() = g;
        this->b() = b;
    }

    T &r = Pixel<T, N>::data[0];
    T &g = Pixel<T, N>::data[1];
    T &b = Pixel<T, N>::data[2];
};

/**
 * @brief A single BGR pixel.
 * Consists of three components of defined size, B, G and R.
 */
template <typename T = uint8_t, size_t N = 3> class BGR : public Pixel<T, N> {
    static_assert(N >= 3, "pixel size must be >= 3");

public:
    BGR() {}
    BGR(T r, T g, T b) {
        this->r() = r;
        this->g() = g;
        this->b() = b;
    }

    T &b = Pixel<T, N>::data[0];
    T &g = Pixel<T, N>::data[1];
    T &r = Pixel<T, N>::data[2];
};

} // namespace pix
} // namespace sph

#endif // SPH_CORE_PIXEL_H
