/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_DETECTOR_LBF_H
#define SPH_FACE_DETECTOR_LBF_H

#include "detector.h"
#include <mutex>
#include <opencv2/face.hpp>
#include <opencv2/opencv.hpp>
#include <seraphim/core/computable.h>
#include <vector>

namespace sph {
namespace face {

class LBPDetector : public IDetector, sph::core::IComputable {
public:
    explicit LBPDetector();
    ~LBPDetector() override;

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

    bool empty() const { return m_impl.empty(); }
    void write(cv::FileStorage &fs) const { return m_impl->write(fs); }

    bool face_cascade_impl(cv::InputArray img, cv::OutputArray ROIs);
    bool load_face_cascade(const std::string &path);
    bool load_facemark_model(const std::string &path);

    bool detect_faces(cv::InputArray img, cv::OutputArray faces) override;
    bool detect_facemarks(cv::InputArray img, cv::InputArray faces,
                          std::vector<Facemarks> &facemarks) override;

    /**
     * @brief Set parameters for various algorithms that are used by this class.
     * @param params Parameters for canny edge detection and hough line transform.
     */
    void set_parameters(const Parameters &params) { m_params = params; }

    bool set_target(const target_t &target) override;

private:
    cv::Ptr<cv::face::FacemarkTrain> m_impl;

    /// parameters used by the image processing pipeline
    Parameters m_params;

    std::unique_ptr<cv::CascadeClassifier> m_face_cascade;
    cv::face::FacemarkLBF::Params m_facemark_params;

    bool m_facemark_model_loaded;

    target_t m_target;
    std::mutex m_target_mutex;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_DETECTOR_LBF_H
