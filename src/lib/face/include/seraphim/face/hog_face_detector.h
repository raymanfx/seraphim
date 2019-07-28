/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_DETECTOR_HOG_H
#define SPH_FACE_DETECTOR_HOG_H

#include <dlib/image_processing/frontal_face_detector.h>
#include <mutex>
#include <seraphim/core/computable.h>
#include <vector>

#include "face_detector.h"

namespace sph {
namespace face {

class HOGFaceDetector : public IFaceDetector, sph::core::IComputable {
public:
    HOGFaceDetector();
    ~HOGFaceDetector() override;

    bool detect_faces(cv::InputArray img, cv::OutputArray faces) override;

    bool set_target(const target_t &target) override;

private:
    dlib::frontal_face_detector m_detector;

    target_t m_target;
    std::mutex m_target_mutex;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_DETECTOR_HOG_H