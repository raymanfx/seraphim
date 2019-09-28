/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_DETECTOR_H
#define SPH_FACE_DETECTOR_H

#include <seraphim/core/image.h>
#include <seraphim/core/polygon.h>
#include <vector>

namespace sph {
namespace face {

class IFaceDetector {
public:
    virtual ~IFaceDetector() = default;

    virtual bool detect_faces(const sph::core::Image &img,
                              std::vector<sph::core::Polygon<int>> &faces) = 0;

    float confidence_threshold() const { return m_confidence_threshold; }
    void set_confidence_threshold(float threshold) { m_confidence_threshold = threshold; }

protected:
    float m_confidence_threshold;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_DETECTOR_H
