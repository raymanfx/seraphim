/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CAR_LANE_DETECTOR_HPP
#define SPH_CAR_LANE_DETECTOR_HPP

#include <seraphim/image.hpp>
#include <seraphim/polygon.hpp>
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
class LaneDetector {
public:
    virtual ~LaneDetector() = default;

    /**
     * @brief Detect lanes in an image.
     * Combines various steps into one image processing pipeline, including preprocessing,
     * edge and line detection and more.
     * @param img Input image.
     * @param lanes Output vector containing polygon lane shapes.
     * @return Whether detection was successful.
     */
    virtual bool detect(const sph::Image &img, std::vector<sph::Polygon<int>> &lanes) = 0;

    /**
     * @brief Define a polygon-shaped region of interest for lane detection.
     * @param polyroi The polygon shape.
     * @return True on success, false otherwise.
     */
    virtual bool set_roi(const sph::Polygon<int> &poly) = 0;
};

} // namespace car
} // namespace sph

#endif // SPH_CAR_LANE_DETECTOR_HPP
