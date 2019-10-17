/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACEMARK_DETECTOR_LBF_H
#define SPH_FACEMARK_DETECTOR_LBF_H

#include <memory>
#include <mutex>
#include <opencv2/face.hpp>
#include <opencv2/opencv.hpp>
#include <seraphim/computable.h>
#include <vector>

#include "face_detector.h"
#include "facemark_detector.h"

namespace sph {
namespace face {

class LBFFacemarkDetector : public FacemarkDetector, sph::Computable {
public:
    LBFFacemarkDetector();

    bool empty() const { return m_facemark_impl.empty(); }
    void write(cv::FileStorage &fs) const { return m_facemark_impl->write(fs); }

    bool load_facemark_model(const std::string &path);

    bool detect(const sph::Image &img, const std::vector<sph::Polygon<int>> &faces,
                std::vector<Facemarks> &facemarks) override;

    bool set_target(Target target) override;

private:
    cv::Ptr<cv::face::FacemarkTrain> m_facemark_impl;
    cv::face::FacemarkLBF::Params m_facemark_params;
    bool face_detector(cv::InputArray img, cv::OutputArray ROIs);

    Target m_target = Target::CPU;
    std::mutex m_target_mutex;
};

} // namespace face
} // namespace sph

#endif // SPH_FACEMARK_DETECTOR_LBF_H
