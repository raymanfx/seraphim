/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_DETECTOR_LBB_H
#define SPH_FACE_DETECTOR_LBB_H

#include <mutex>
#include <opencv2/face.hpp>
#include <opencv2/opencv.hpp>
#include <seraphim/core/computable.h>
#include <vector>

#include "face_detector.h"

namespace sph {
namespace face {

class LBPFaceDetector : public IFaceDetector, sph::core::IComputable {
public:
    LBPFaceDetector();
    ~LBPFaceDetector() override;

    /**
     * @brief Parameters used by algorithms in the image processing pipeline.
     */
    struct Parameters {
        // casecade classifier params
        double cascade_scale_factor;
        int cascade_min_neighbours;
        int cascade_flags;
        cv::Size cascade_min_size;
    };

    bool empty() const { return m_face_cascade.empty(); }

    bool load_face_cascade(const std::string &path);

    bool detect_faces(const sph::core::Image &img,
                      std::vector<sph::core::Polygon<int>> &faces) override;

    /**
     * @brief Set parameters for various algorithms that are used by this class.
     * @param params Parameters for canny edge detection and hough line transform.
     */
    void set_parameters(const Parameters &params) { m_params = params; }

    bool set_target(const target_t &target) override;

private:
    /// parameters used by the image processing pipeline
    Parameters m_params;

    cv::CascadeClassifier m_face_cascade;
    bool face_cascade_impl(cv::InputArray img, cv::OutputArray ROIs);

    target_t m_target;
    std::mutex m_target_mutex;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_DETECTOR_LBB_H
