/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <utils.h>

#include "detector_lbp_service.h"

using namespace sph::face;

LBPDetectorService::LBPDetectorService(sph::face::LBPDetector *detector) {
    m_detector = detector;
}

bool LBPDetectorService::handle_request(const Seraphim::Request &req, Seraphim::Response &res) {
    if (!req.has_face() || !req.face().has_detector()) {
        return false;
    }

    if (req.face().detector().has_detection()) {
        return handle_detection_request(
            req.face().detector().detection(),
            *res.mutable_face()->mutable_detector()->mutable_detection());
    }

    return false;
}

bool LBPDetectorService::handle_detection_request(
    const Seraphim::Face::Detector::DetectionRequest &req,
    Seraphim::Face::Detector::DetectionResponse &res) {
    cv::Mat image;
    std::vector<cv::Rect> faces;
    std::vector<std::vector<cv::Point2f>> facemarks;
    cv::Rect2i roi;

    if (!sph::backend::Image2DtoMat(req.image(), image)) {
        return false;
    }

    roi = cv::Rect2i(0, 0, image.cols, image.rows);

    if (req.has_roi()) {
        roi.x = req.roi().x();
        roi.y = req.roi().y();
        roi.width = req.roi().w();
        roi.height = req.roi().h();
    }

    m_detector->detect_faces(image(roi), faces);
    m_detector->detect_facemarks(image(roi), faces, facemarks);

    for (size_t i = 0; i < faces.size(); i++) {
        Seraphim::Types::Region2D *face = res.add_faces();
        face->set_x(faces[i].x);
        face->set_y(faces[i].y);
        face->set_w(faces[i].width);
        face->set_h(faces[i].height);
    }

    for (size_t i = 0; i < facemarks.size(); i++) {
        Seraphim::Face::Detector::Facemarks *facemarks_ = res.add_facemarks();
        Seraphim::Types::PointSet2D *points = facemarks_->add_pointsets();
        for (size_t j = facemark_LUT[LBPDetector::FACEMARK_JAW].first;
             j <= facemark_LUT[LBPDetector::FACEMARK_JAW].second; j++) {
            facemarks_->add_landmarks(Seraphim::Face::Detector::Facemarks::JAW);
            Seraphim::Types::Point2D *point = points->add_points();
            point->set_x(static_cast<int>(facemarks.at(i).at(j).x));
            point->set_y(static_cast<int>(facemarks.at(i).at(j).y));
        }
        for (size_t j = facemark_LUT[LBPDetector::FACEMARK_RIGHT_EYEBROW].first;
             j <= facemark_LUT[LBPDetector::FACEMARK_RIGHT_EYEBROW].second; j++) {
            facemarks_->add_landmarks(Seraphim::Face::Detector::Facemarks::RIGHT_EYEBROW);
            Seraphim::Types::Point2D *point = points->add_points();
            point->set_x(static_cast<int>(facemarks.at(i).at(j).x));
            point->set_y(static_cast<int>(facemarks.at(i).at(j).y));
        }
        for (size_t j = facemark_LUT[LBPDetector::FACEMARK_LEFT_EYEBROW].first;
             j <= facemark_LUT[LBPDetector::FACEMARK_LEFT_EYEBROW].second; j++) {
            facemarks_->add_landmarks(Seraphim::Face::Detector::Facemarks::LEFT_EYEBROW);
            Seraphim::Types::Point2D *point = points->add_points();
            point->set_x(static_cast<int>(facemarks.at(i).at(j).x));
            point->set_y(static_cast<int>(facemarks.at(i).at(j).y));
        }
        for (size_t j = facemark_LUT[LBPDetector::FACEMARK_NOSE].first;
             j <= facemark_LUT[LBPDetector::FACEMARK_NOSE].second; j++) {
            facemarks_->add_landmarks(Seraphim::Face::Detector::Facemarks::NOSE);
            Seraphim::Types::Point2D *point = points->add_points();
            point->set_x(static_cast<int>(facemarks.at(i).at(j).x));
            point->set_y(static_cast<int>(facemarks.at(i).at(j).y));
        }
        for (size_t j = facemark_LUT[LBPDetector::FACEMARK_RIGHT_EYE].first;
             j <= facemark_LUT[LBPDetector::FACEMARK_RIGHT_EYE].second; j++) {
            facemarks_->add_landmarks(Seraphim::Face::Detector::Facemarks::RIGHT_EYE);
            Seraphim::Types::Point2D *point = points->add_points();
            point->set_x(static_cast<int>(facemarks.at(i).at(j).x));
            point->set_y(static_cast<int>(facemarks.at(i).at(j).y));
        }
        for (size_t j = facemark_LUT[LBPDetector::FACEMARK_LEFT_EYE].first;
             j <= facemark_LUT[LBPDetector::FACEMARK_LEFT_EYE].second; j++) {
            facemarks_->add_landmarks(Seraphim::Face::Detector::Facemarks::LEFT_EYE);
            Seraphim::Types::Point2D *point = points->add_points();
            point->set_x(static_cast<int>(facemarks.at(i).at(j).x));
            point->set_y(static_cast<int>(facemarks.at(i).at(j).y));
        }
        for (size_t j = facemark_LUT[LBPDetector::FACEMARK_MOUTH].first;
             j <= facemark_LUT[LBPDetector::FACEMARK_MOUTH].second; j++) {
            facemarks_->add_landmarks(Seraphim::Face::Detector::Facemarks::MOUTH);
            Seraphim::Types::Point2D *point = points->add_points();
            point->set_x(static_cast<int>(facemarks.at(i).at(j).x));
            point->set_y(static_cast<int>(facemarks.at(i).at(j).y));
        }
    }

    return true;
}
