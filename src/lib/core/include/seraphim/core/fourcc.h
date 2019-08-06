/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_FOURCC_H
#define SPH_CORE_FOURCC_H

#include <cstdint>

namespace sph {
namespace core {

static inline constexpr uint32_t fourcc(const char &a, const char &b, const char &c,
                                        const char &d) {
    return ((static_cast<uint32_t>(a) << 0) | (static_cast<uint32_t>(b) << 8) |
            (static_cast<uint32_t>(c) << 16) | (static_cast<uint32_t>(d) << 24));
}

} // namespace core
} // namespace sph

#endif // SPH_CORE_FOURCC_H
