/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CAR_LANE_DETECTOR_SERVICE_H
#define SPH_CAR_LANE_DETECTOR_SERVICE_H

#include <LaneDetector.pb.h>
#include <seraphim/car/lane_detector.h>

#include "../service.h"

namespace sph {
namespace car {

class LaneDetectorService : public sph::backend::IService {
public:
    explicit LaneDetectorService(std::shared_ptr<sph::car::ILaneDetector> detector);

    bool handle_request(const Seraphim::Request &req, Seraphim::Response &res) override;
    bool handle_detection_request(const Seraphim::Car::LaneDetector::DetectionRequest &req,
                                  Seraphim::Car::LaneDetector::DetectionResponse &res);

private:
    std::shared_ptr<sph::car::ILaneDetector> m_detector;
};

} // namespace car
} // namespace sph

#endif // SPH_CAR_LANE_DETECTOR_SERVICE_H
