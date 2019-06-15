/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_BACKEND_UTILS_H
#define SPH_BACKEND_UTILS_H

#include <Types.pb.h>
#include <opencv2/core/mat.hpp>

namespace sph {
namespace backend {

/**
 * @brief fourcc Compute the four character code for a string.
 * @param c1 e.g. 'M'
 * @param c2 e.g. 'J'
 * @param c3 e.g. 'P'
 * @param c4 e.g. 'G'
 * @return The four character code.
 */
inline constexpr uint32_t fourcc(const char &c1, const char &c2, const char &c3, const char &c4) {
    return ((static_cast<uint32_t>(c1) << 0) | (static_cast<uint32_t>(c2) << 8) |
            (static_cast<uint32_t>(c3) << 16) | (static_cast<uint32_t>(c4) << 24));
}

/**
 * @brief Image2DtoMat Convert arbitrary image data to matrix type.
 * @param img Input image from an IPC message.
 * @param mat Output matrix type that wraps the image data. A deep copy of the buffer is performed.
 * @return True on success, false otherwise.
 */
bool Image2DtoMat(const Seraphim::Types::Image2D &img, cv::Mat &mat);

} // namespace backend
} // namespace sph

#endif // SPH_BACKEND_UTILS_H
