/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACEMARK_DETECTOR_H
#define SPH_FACEMARK_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <vector>

namespace sph {
namespace face {

class IFacemarkDetector {
public:
    virtual ~IFacemarkDetector() = default;

    /**
     * @brief Facemark type.
     */
    enum class FacemarkType { JAW, RIGHT_EYEBROW, LEFT_EYEBROW, NOSE, RIGHT_EYE, LEFT_EYE, MOUTH };

    /**
     * @brief Struct describing the landmarks of a face.
     */
    struct Facemarks {
        /// all the point positions for each facemark of a face
        std::vector<std::pair<FacemarkType, std::vector<cv::Point>>> landmarks;
    };

    virtual bool detect_facemarks(cv::InputArray img, cv::InputArray faces,
                                  std::vector<Facemarks> &facemarks) = 0;
};

} // namespace face
} // namespace sph

#endif // SPH_FACEMARK_DETECTOR_H
