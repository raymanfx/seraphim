/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACEMARK_DETECTOR_SERVICE_H
#define SPH_FACEMARK_DETECTOR_SERVICE_H

#include <FacemarkDetector.pb.h>
#include <seraphim/face/face_detector.h>
#include <seraphim/face/facemark_detector.h>

#include "../service.h"

namespace sph {
namespace face {

class FacemarkDetectorService : public sph::backend::IService {
public:
    FacemarkDetectorService(std::shared_ptr<sph::face::IFaceDetector> face_detector,
                            std::shared_ptr<sph::face::IFacemarkDetector> facemark_detector);

    bool handle_request(const Seraphim::Request &req, Seraphim::Response &res) override;
    bool handle_detection_request(const Seraphim::Face::FacemarkDetector::DetectionRequest &req,
                                  Seraphim::Face::FacemarkDetector::DetectionResponse &res);

private:
    std::shared_ptr<sph::face::IFaceDetector> m_face_detector;
    std::shared_ptr<sph::face::IFacemarkDetector> m_facemark_detector;
};

} // namespace face
} // namespace sph

#endif // SPH_FACEMARK_DETECTOR_SERVICE_H
