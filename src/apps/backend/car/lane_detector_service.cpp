/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <seraphim/image.h>
#include <seraphim/polygon.h>
#include <utils.h>

#include "lane_detector_service.h"

using namespace sph;
using namespace sph::car;

LaneDetectorService::LaneDetectorService(std::shared_ptr<sph::car::LaneDetector> detector) {
    m_detector = detector;
}

bool LaneDetectorService::handle_request(const Seraphim::Request &req, Seraphim::Response &res) {
    if (req.inner().Is<Seraphim::Car::LaneDetector::DetectionRequest>()) {
        Seraphim::Car::LaneDetector::DetectionRequest inner_req;
        Seraphim::Car::LaneDetector::DetectionResponse inner_res;

        req.inner().UnpackTo(&inner_req);
        if (handle_detection_request(inner_req, inner_res)) {
            res.mutable_inner()->PackFrom(inner_res);
            return true;
        }
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
