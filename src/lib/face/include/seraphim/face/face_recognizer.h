/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_RECOGNIZER_H
#define SPH_FACE_RECOGNIZER_H

#include <seraphim/image.h>
#include <seraphim/polygon.h>
#include <vector>

namespace sph {
namespace face {

class FaceRecognizer {
public:
    virtual ~FaceRecognizer() = default;

    struct Prediction {
        int label;
        sph::Polygon<int> poly;
        double confidence;
    };

    virtual void train(const std::vector<sph::BufferedImage> &imgs,
                       const std::vector<int> &labels) = 0;
    virtual bool predict(const sph::Image &img, std::vector<Prediction> &preds) = 0;
    virtual void update(const std::vector<sph::BufferedImage> &imgs, const std::vector<int> &labels,
                        bool invalidate = false) = 0;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_RECOGNIZER_H
