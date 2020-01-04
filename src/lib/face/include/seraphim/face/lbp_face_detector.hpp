/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_DETECTOR_LBB_HPP
#define SPH_FACE_DETECTOR_LBB_HPP

#include <mutex>
#include <opencv2/face.hpp>
#include <opencv2/opencv.hpp>
#include <seraphim/computable.hpp>
#include <vector>

#include "face_detector.hpp"

namespace sph {
namespace face {

class LBPFaceDetector : public FaceDetector, sph::Computable {
public:
    LBPFaceDetector() = default;

    /**
     * @brief Parameters used by algorithms in the image processing pipeline.
     */
    struct Parameters {
        // casecade classifier params
        double cascade_scale_factor = 1.1;
        int cascade_min_neighbours = 3;
        int cascade_flags = cv::CASCADE_SCALE_IMAGE;
        cv::Size cascade_min_size = cv::Size(30, 30);
    };

    bool empty() const { return m_face_cascade.empty(); }

    bool load_face_cascade(const std::string &path);

    bool detect(const sph::Image &img, std::vector<sph::Polygon<int>> &faces) override;

    /**
     * @brief Set parameters for various algorithms that are used by this class.
     * @param params Parameters for canny edge detection and hough line transform.
     */
    void set_parameters(const Parameters &params) { m_params = params; }

    bool set_target(Target target) override;

private:
    /// parameters used by the image processing pipeline
    Parameters m_params;

    cv::CascadeClassifier m_face_cascade;
    bool face_cascade_impl(cv::InputArray img, cv::OutputArray ROIs);

    Target m_target = Target::CPU;
    std::mutex m_target_mutex;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_DETECTOR_LBB_HPP
