/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <seraphim/core/image.h>
#include <seraphim/core/polygon.h>
#include <utils.h>

#include "lane_detector_service.h"

using namespace sph;
using namespace sph::car;

LaneDetectorService::LaneDetectorService(std::shared_ptr<sph::car::ILaneDetector> detector) {
    m_detector = detector;
}

bool LaneDetectorService::handle_request(const Seraphim::Request &req, Seraphim::Response &res) {
    if (!req.has_car() || !req.car().has_detector()) {
        return false;
    }

    if (req.car().detector().has_detection()) {
        return handle_detection_request(
            req.car().detector().detection(),
            *res.mutable_car()->mutable_detector()->mutable_detection());
    }

    return false;
}

bool LaneDetectorService::handle_detection_request(
    const Seraphim::Car::LaneDetector::DetectionRequest &req,
    Seraphim::Car::LaneDetector::DetectionResponse &res) {
    Image image;
    Polygon<int> polyroi;
    std::vector<Polygon<int>> lanes;

    if (!sph::backend::Image2DtoImage(req.image(), image)) {
        return false;
    }

    if (req.has_polyroi()) {
        for (const auto &point : req.polyroi().points()) {
            polyroi.add_point(Point2i(point.x(), point.y()));
        }
        m_detector->set_roi(polyroi);
    } else {
        m_detector->set_roi({});
    }

    m_detector->detect(image, lanes);
    for (const auto &lane : lanes) {
        Seraphim::Car::LaneDetector::Lane *lane_ = res.add_lanes();
        lane_->mutable_bottomleft()->set_x(lane.points()[0].x);
        lane_->mutable_bottomleft()->set_y(lane.points()[0].y);
        lane_->mutable_topleft()->set_x(lane.points()[1].x);
        lane_->mutable_topleft()->set_y(lane.points()[1].y);
        lane_->mutable_topright()->set_x(lane.points()[2].x);
        lane_->mutable_topright()->set_y(lane.points()[2].y);
        lane_->mutable_bottomright()->set_x(lane.points()[3].x);
        lane_->mutable_bottomright()->set_y(lane.points()[3].y);
    }

    return true;
}
