/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CAR_LANE_DETECTOR_H
#define SPH_CAR_LANE_DETECTOR_H

#include <seraphim/core/image.h>
#include <seraphim/core/polygon.h>
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
     * @brief Detect lanes in an image.
     * Combines various steps into one image processing pipeline, including preprocessing,
     * edge and line detection and more.
     * @param img Input image.
     * @param lanes Output vector containing polygon lane shapes.
     * @return Whether detection was successful.
     */
    virtual bool detect(const sph::core::Image &img, std::vector<sph::core::Polygon<>> &lanes) = 0;

    /**
     * @brief Define a polygon-shaped region of interest for lane detection.
     * @param polyroi The polygon shape.
     * @return True on success, false otherwise.
     */
    virtual bool set_roi(const sph::core::Polygon<> &poly) = 0;

protected:
    ILaneDetector() = default;
};

} // namespace car
} // namespace sph

#endif // SPH_CAR_LANE_DETECTOR_H
