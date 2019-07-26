/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_DETECTOR_LBF_H
#define SPH_FACE_DETECTOR_LBF_H

#include "detector.h"
#include <opencv2/face.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

namespace sph {
namespace face {

static const std::pair<size_t, size_t> facemark_LUT[] = {
    { 0, 16 },  // FACEMARK_JAW
    { 17, 21 }, // FACEMARK_RIGHT_EYEBROW
    { 22, 26 }, // FACEMARK_LEFT_EYEBROW
    { 27, 35 }, // FACEMARK_NOSE
    { 36, 41 }, // FACEMARK_RIGHT_EYE
    { 42, 47 }, // FACEMARK_LEFT_EYE
    { 48, 67 }  // FACEMARK_MOUTH
};

class LBPDetector : public IDetector {
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

    typedef enum {
        FACEMARK_JAW = 0,
        FACEMARK_RIGHT_EYEBROW,
        FACEMARK_LEFT_EYEBROW,
        FACEMARK_NOSE,
        FACEMARK_RIGHT_EYE,
        FACEMARK_LEFT_EYE,
        FACEMARK_MOUTH,
        FACEMARK_MAX
    } facemark_t;

    bool empty() const { return m_impl.empty(); }
    void write(cv::FileStorage &fs) const { return m_impl->write(fs); }

    bool face_cascade_impl(cv::InputArray img, cv::OutputArray ROIs);
    bool load_face_cascade(const std::string &path);
    bool load_facemark_model(const std::string &path);

    bool detect_faces(cv::InputArray img, cv::OutputArray faces) override;
    bool detect_facemarks(cv::InputArray img, cv::InputArray faces,
                          cv::OutputArrayOfArrays facemarks) override;

    bool find_eyes(const std::vector<cv::Point2f> &facemarks, std::vector<cv::Point2f> &eyes) const;

    /**
     * @brief Set parameters for various algorithms that are used by this class.
     * @param params Parameters for canny edge detection and hough line transform.
     */
    void set_parameters(const Parameters &params) { m_params = params; }

private:
    cv::Ptr<cv::face::FacemarkTrain> m_impl;

    /// parameters used by the image processing pipeline
    Parameters m_params;

    std::unique_ptr<cv::CascadeClassifier> m_face_cascade;
    cv::face::FacemarkLBF::Params m_facemark_params;

    bool m_facemark_model_loaded;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_DETECTOR_LBF_H