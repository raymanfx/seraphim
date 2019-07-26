/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include "lane_detector_service.h"

using namespace sph::car;

LaneDetectorService::LaneDetectorService(sph::car::ILaneDetector *detector) {
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
    cv::Mat image;
    cv::Rect2i roi;
    std::vector<sph::car::ILaneDetector::Lane> lanes;

    image = cv::Mat(req.image().rows(), req.image().cols(), req.image().type(),
                    const_cast<char *>(req.image().data().c_str()));

    roi = cv::Rect2i(0, 0, image.cols, image.rows);

    if (req.has_roi()) {
        roi.x = req.roi().x();
        roi.y = req.roi().y();
        roi.width = req.roi().w();
        roi.height = req.roi().h();
    }

    m_detector->detect(image(roi), lanes);
    for (const auto &lane : lanes) {
        Seraphim::Car::LaneDetector::Lane *lane_ = res.add_lanes();
        lane_->mutable_bottomleft()->set_x(lane.bottomLeft.x);
        lane_->mutable_bottomleft()->set_y(lane.bottomLeft.y);
        lane_->mutable_topleft()->set_x(lane.topLeft.x);
        lane_->mutable_topleft()->set_y(lane.topLeft.y);
        lane_->mutable_topright()->set_x(lane.topRight.x);
        lane_->mutable_topright()->set_y(lane.topRight.y);
        lane_->mutable_bottomright()->set_x(lane.bottomRight.x);
        lane_->mutable_bottomright()->set_y(lane.bottomRight.y);
    }

    return true;
}
