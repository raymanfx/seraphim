/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_DETECTOR_H
#define SPH_FACE_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <vector>

namespace sph {
namespace face {

class IDetector {
public:
    virtual ~IDetector() = default;

    /**
     * @brief Facemark type.
     */
    enum class FacemarkType {
        JAW ,
        RIGHT_EYEBROW,
        LEFT_EYEBROW,
        NOSE,
        RIGHT_EYE,
        LEFT_EYE,
        MOUTH
    };

    /**
     * @brief Struct describing the landmarks of a face.
     */
    struct Facemarks {
        /// all the point positions for each facemark of a face
        std::vector<std::pair<FacemarkType, std::vector<cv::Point>>> landmarks;
    };

    virtual bool detect_faces(cv::InputArray img, cv::OutputArray faces) = 0;
    virtual bool detect_facemarks(cv::InputArray img, cv::InputArray faces,
                                  std::vector<Facemarks> &facemarks) = 0;

    float confidence_threshold() const { return m_confidence_threshold; }
    void set_confidence_threshold(const float &threshold) { m_confidence_threshold = threshold; }

protected:
    IDetector() = default;

    float m_confidence_threshold;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_DETECTOR_H
