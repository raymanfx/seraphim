/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_RECOGNIZER_H
#define SPH_FACE_RECOGNIZER_H

#include <opencv2/opencv.hpp>
#include <vector>

namespace sph {
namespace face {

class IRecognizer {
public:
    virtual ~IRecognizer() = default;

    struct Prediction {
        int label;
        cv::Rect rect;
        double confidence;
    };

    virtual void train(cv::InputArrayOfArrays imgs, const std::vector<int> &labels) = 0;
    virtual bool predict(cv::InputArray img, std::vector<Prediction> &preds) = 0;
    virtual void update(cv::InputArrayOfArrays imgs, const std::vector<int> &labels,
                        bool invalidate = false) = 0;

protected:
    IRecognizer() = default;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_RECOGNIZER_H
