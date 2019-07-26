/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_OBJECT_CLASSIFIER_H
#define SPH_OBJECT_CLASSIFIER_H

#include <opencv2/opencv.hpp>
#include <vector>

namespace sph {
namespace object {

/**
 * @brief Object classifier interface.
 *
 * Derive from this class to implement an object classifier.
 * The interface provides a basic prediction data type and requires you to
 * implement the abstract @ref predict method.
 */
class Classifier {
public:
    virtual ~Classifier() = default;

    /**
     * @brief Object prediction data struct.
     */
    struct Prediction {
        /// class id (taken from the dataset that was used in @ref predict)
        int class_id;
        /// bounding rectangle, relative to the input image
        cv::Rect rect;
        /// confidence (value between 0 and 1)
        float confidence;
    };

    /**
     * @brief Predict object classes and locations in an image.
     * @param img Input image.
     * @param preds Output vector containing @ref Prediction instances.
     * @return Whether prediction was successful.
     */
    virtual bool predict(cv::InputArray img, std::vector<Prediction> &preds) = 0;

protected:
    Classifier() = default;
};

} // namespace object
} // namespace sph

#endif // SPH_OBJECT_CLASSIFIER_H