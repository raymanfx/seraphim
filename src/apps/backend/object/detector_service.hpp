/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_OBJECT_DETECTOR_SERVICE_HPP
#define SPH_OBJECT_DETECTOR_SERVICE_HPP

#include <ObjectDetector.pb.h>
#include <seraphim/object/detector.hpp>

#include "../service.hpp"

namespace sph {
namespace object {

class DetectorService : public sph::backend::Service {
public:
    explicit DetectorService(std::shared_ptr<sph::object::Detector> recognizer);

    bool handle_request(const Seraphim::Request &req, Seraphim::Response &res) override;
    bool handle_detection_request(const Seraphim::Object::Detector::DetectionRequest &req,
                                  Seraphim::Object::Detector::DetectionResponse &res);

private:
    std::shared_ptr<sph::object::Detector> m_recognizer;
};

} // namespace object
} // namespace sph

#endif // SPH_OBJECT_DETECTOR_SERVICE_HPP
