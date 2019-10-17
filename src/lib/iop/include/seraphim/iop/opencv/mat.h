/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IOP_CV_MAT_H
#define SPH_IOP_CV_MAT_H

#include <opencv2/core.hpp>
#include <seraphim/image.h>

namespace sph {
namespace iop {
namespace cv {

/**
 * @brief OpenCV Mat facility.
 *
 * All conversion to or from the OpenCV Matrix type go through this facility.
 * The most obvious use of this class is the bidirectional conversion between sph::Image and
 * cv::Mat.
 */
class MatFacility {
public:
    /**
     * @brief Singleton class instance.
     * @return The single, static instance of this class.
     */
    static MatFacility &Instance() {
        // Guaranteed to be destroyed, instantiated on first use.
        static MatFacility instance;
        return instance;
    }

    // Remove copy and assignment constructors.
    MatFacility(MatFacility const &) = delete;
    void operator=(MatFacility const &) = delete;

    /**
     * @brief Convert a Seraphim Image to an OpenCV Matrix.
     * @param img The source image.
     * @return The converted Matrix, which is empty if the conversion failed.
     */
    static ::cv::Mat from_image(const Image &img);

    /**
     * @brief Convert an OpenCV Matrix to a Seraphim Image.
     * @param mat The source Matrix.
     * @return The converted Image, which is empty if the conversion failed.
     */
    static sph::Image to_image(const ::cv::Mat &mat);

private:
    MatFacility() = default;
};

} // namespace cv
} // namespace iop
} // namespace sph

#endif // SPH_IOP_CV_MAT_H
