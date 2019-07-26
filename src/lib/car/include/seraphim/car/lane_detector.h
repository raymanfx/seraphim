/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CAR_LANE_DETECTOR_H
#define SPH_CAR_LANE_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <vector>

namespace sph {
namespace car {

/**
 * @brief Lane detector interface.
 *
 * Derive from this class to implement a lane detector.
 * The interface provides a basic lane data type and requires you to
 * implement the abstract @ref detect method.
 */
class ILaneDetector {
public:
    virtual ~ILaneDetector() = default;

    /**
     * @brief Lane data struct.
     */
    struct Lane {
        /// top left point of the left edge
        cv::Point topLeft;
        /// top right point of the right edge
        cv::Point topRight;
        /// bottom right point of the right edge
        cv::Point bottomRight;
        /// bottom left point of the left edge
        cv::Point bottomLeft;
    };

    /**
     * @brief Detect lanes in an image.
     * Combines various steps into one image processing pipeline, including preprocessing,
     * edge and line detection and more.
     * @param img Input image.
     * @param lanes Output vector containing @ref Lane instances.
     * @return Whether detection was successful.
     */
    virtual bool detect(cv::InputArray img, std::vector<Lane> &lanes) = 0;

protected:
    ILaneDetector() = default;
};

} // namespace car
} // namespace sph

#endif // SPH_CAR_LANE_DETECTOR_H
