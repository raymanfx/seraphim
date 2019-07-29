/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_RECOGNIZER_H
#define SPH_FACE_RECOGNIZER_H

#include <seraphim/core/image.h>
#include <seraphim/core/polygon.h>
#include <vector>

namespace sph {
namespace face {

class IFaceRecognizer {
public:
    virtual ~IFaceRecognizer() = default;

    struct Prediction {
        int label;
        sph::core::Polygon<> poly;
        double confidence;
    };

    virtual void train(const std::vector<sph::core::Image> &imgs,
                       const std::vector<int> &labels) = 0;
    virtual bool predict(const sph::core::Image &img, std::vector<Prediction> &preds) = 0;
    virtual void update(const std::vector<sph::core::Image> &imgs, const std::vector<int> &labels,
                        bool invalidate = false) = 0;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_RECOGNIZER_H
