/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IOP_CV_MAT_HPP
#define SPH_IOP_CV_MAT_HPP

#include <opencv2/core.hpp>
#include <seraphim/image.hpp>
#include <seraphim/matrix.hpp>

namespace sph {
namespace iop {
namespace cv {

/**
 * @brief Convert a Seraphim Matrix to an OpenCV Matrix.
 * @param mat The source matrix.
 * @return The converted Matrix, which is empty if the conversion failed.
 */
template <typename T>::cv::Mat_<T> from_matrix(const Matrix<T> &mat);

/**
 * @brief Convert an OpenCV Matrix to a Seraphim Matrix.
 * @param mat The source Matrix.
 * @return The converted Matrix, which is empty if the conversion failed.
 */
template <typename T> Matrix<T> to_matrix(const ::cv::Mat_<T> &mat);

/**
 * @brief Convert a Seraphim Image to an OpenCV Matrix.
 * @param img The source image.
 * @return The converted Matrix, which is empty if the conversion failed.
 */
::cv::Mat from_image(const Image &img);

/**
 * @brief Convert an OpenCV Matrix to a Seraphim Image.
 * @param mat The source Matrix.
 * @return The converted Image, which is empty if the conversion failed.
 */
CoreImage to_image(const ::cv::Mat &mat);

} // namespace cv
} // namespace iop
} // namespace sph

#endif // SPH_IOP_CV_MAT_HPP