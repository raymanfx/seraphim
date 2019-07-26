/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_DETECTOR_SERVICE_H
#define SPH_FACE_DETECTOR_SERVICE_H

#include <FaceDetector.pb.h>
#include <seraphim/face/detector.h>

#include "../service.h"

namespace sph {
namespace face {

class DetectorService : public sph::backend::IService {
public:
    explicit DetectorService(std::shared_ptr<sph::face::IDetector> detector);

    bool handle_request(const Seraphim::Request &req, Seraphim::Response &res) override;
    bool handle_detection_request(const Seraphim::Face::Detector::DetectionRequest &req,
                                  Seraphim::Face::Detector::DetectionResponse &res);

private:
    std::shared_ptr<sph::face::IDetector> m_detector;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_DETECTOR_SERVICE_H
