/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_OBJECT_DETECTOR_H
#define SPH_OBJECT_DETECTOR_H

#include <seraphim/image.h>
#include <seraphim/polygon.h>
#include <vector>

namespace sph {
namespace object {

/**
 * @brief Object detector interface.
 *
 * Derive from this class to implement an object detector.
 * The interface provides a basic prediction data type and requires you to
 * implement the abstract @ref predict method.
 */
class Detector {
public:
    virtual ~Detector() = default;

    /**
     * @brief Object prediction data struct.
     */
    struct Prediction {
        /// class id (taken from the dataset that was used in @ref predict)
        int class_id = -1;
        /// bounding polygon, relative to the input image
        sph::Polygon<int> poly;
        /// confidence (value between 0 and 1)
        float confidence = 0.0f;
    };

    /**
     * @brief Predict object classes and locations in an image.
     * @param img Input image.
     * @param preds Output vector containing @ref Prediction instances.
     * @return Whether prediction was successful.
     */
    virtual bool predict(const sph::Image &img, std::vector<Prediction> &preds) = 0;
};

} // namespace object
} // namespace sph

#endif // SPH_OBJECT_DETECTOR_H
