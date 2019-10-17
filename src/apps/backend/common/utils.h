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
#include <seraphim/image.h>

namespace sph {
namespace backend {

/**
 * @brief Image2DtoImage Convert arbitrary image data to our internal image representation.
 * @param src Input image from an IPC message.
 * @param dst Output image type that wraps the image data. No copying is performed.
 * @return True on success, false otherwise.
 */
bool Image2DtoImage(const Seraphim::Types::Image2D &src, sph::Image &dst);

/**
 * @brief Image2DtoMat Convert arbitrary image data to matrix type.
 * @param src Input image from an IPC message.
 * @param dst Output matrix type that wraps the image data. A deep copy of the buffer is performed.
 * @return True on success, false otherwise.
 */
bool Image2DtoMat(const Seraphim::Types::Image2D &src, cv::Mat &dst);

} // namespace backend
} // namespace sph

#endif // SPH_BACKEND_UTILS_H
