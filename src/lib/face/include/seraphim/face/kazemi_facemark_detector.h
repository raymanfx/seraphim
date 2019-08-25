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
#include <seraphim/core/computable.h>
#include <vector>

#include "face_detector.h"
#include "facemark_detector.h"

namespace sph {
namespace face {

class KazemiFacemarkDetector : public IFacemarkDetector, sph::core::IComputable {
public:
    explicit KazemiFacemarkDetector(std::shared_ptr<IFaceDetector> detector)
        : m_detector(detector) {}

    bool load_facemark_model(const std::string &path);

    bool detect_facemarks(const sph::core::Image &img,
                          const std::vector<sph::core::Polygon<int>> &faces,
                          std::vector<Facemarks> &facemarks) override;

    bool set_target(const target_t &target) override;

private:
    std::shared_ptr<IFaceDetector> m_detector;

    dlib::shape_predictor m_predictor;

    target_t m_target = TARGET_CPU;
    std::mutex m_target_mutex;
};

} // namespace face
} // namespace sph

#endif // SPH_FACEMARK_DETECTOR_KAZEMI_H
