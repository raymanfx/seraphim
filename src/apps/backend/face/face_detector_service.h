/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_DETECTOR_SERVICE_H
#define SPH_FACE_DETECTOR_SERVICE_H

#include <FaceDetector.pb.h>
#include <seraphim/face/face_detector.h>

#include "../service.h"

namespace sph {
namespace face {

class FaceDetectorService : public sph::backend::Service {
public:
    explicit FaceDetectorService(std::shared_ptr<sph::face::FaceDetector> detector);

    bool handle_request(const Seraphim::Request &req, Seraphim::Response &res) override;
    bool handle_detection_request(const Seraphim::Face::FaceDetector::DetectionRequest &req,
                                  Seraphim::Face::FaceDetector::DetectionResponse &res);

private:
    std::shared_ptr<sph::face::FaceDetector> m_detector;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_DETECTOR_SERVICE_H
