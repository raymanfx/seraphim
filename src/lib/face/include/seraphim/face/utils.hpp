/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_UTILS_HPP
#define SPH_FACE_UTILS_HPP

#include <vector>

#include <opencv2/opencv.hpp>

namespace sph {
namespace face {

extern double align_face(cv::InputOutputArray image, std::vector<cv::Point2f> &eyes);

} // namespace face
} // namespace sph

#endif // SPH_FACE_UTILS_HPP
