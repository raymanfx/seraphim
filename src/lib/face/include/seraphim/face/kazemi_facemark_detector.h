/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACEMARK_DETECTOR_KAZEMI_H
#define SPH_FACEMARK_DETECTOR_KAZEMI_H

#include <dlib/image_processing.h>
#include <memory>
#include <mutex>
#include <seraphim/computable.h>
#include <vector>

#include "face_detector.h"
#include "facemark_detector.h"

namespace sph {
namespace face {

class KazemiFacemarkDetector : public FacemarkDetector, sph::Computable {
public:
    KazemiFacemarkDetector() = default;

    bool load_facemark_model(const std::string &path);

    bool detect(const sph::Image &img, const std::vector<sph::Polygon<int>> &faces,
                std::vector<Facemarks> &facemarks) override;

    bool set_target(Target target) override;

private:
    dlib::shape_predictor m_predictor;

    Target m_target = Target::CPU;
    std::mutex m_target_mutex;
};

} // namespace face
} // namespace sph

#endif // SPH_FACEMARK_DETECTOR_KAZEMI_H
