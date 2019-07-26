/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <utils.h>

#include "detector_lbp_service.h"

using namespace sph::face;

LBPDetectorService::LBPDetectorService(std::shared_ptr<sph::face::LBPDetector> detector) {
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
    std::vector<sph::face::IDetector::Facemarks> facemarks;
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

    for (const auto &face : facemarks) {
        for (const auto &landmark : face.landmarks) {
            Seraphim::Face::Detector::Facemarks::Landmark type;
            switch (landmark.first) {
            case IDetector::FacemarkType::JAW:
                type = Seraphim::Face::Detector::Facemarks::JAW;
                break;
            case IDetector::FacemarkType::RIGHT_EYEBROW:
                type = Seraphim::Face::Detector::Facemarks::RIGHT_EYEBROW;
                break;
            case IDetector::FacemarkType::LEFT_EYEBROW:
                type = Seraphim::Face::Detector::Facemarks::LEFT_EYEBROW;
                break;
            case IDetector::FacemarkType::NOSE:
                type = Seraphim::Face::Detector::Facemarks::NOSE;
                break;
            case IDetector::FacemarkType::RIGHT_EYE:
                type = Seraphim::Face::Detector::Facemarks::RIGHT_EYE;
                break;
            case IDetector::FacemarkType::LEFT_EYE:
                type = Seraphim::Face::Detector::Facemarks::LEFT_EYE;
                break;
            case IDetector::FacemarkType::MOUTH:
                type = Seraphim::Face::Detector::Facemarks::MOUTH;
                break;
            default:
                // unknown facemark
                continue;
            }

            Seraphim::Face::Detector::Facemarks *facemarks_ = res.add_facemarks();
            Seraphim::Types::PointSet2D *points = facemarks_->add_pointsets();

            for (const auto &landmark_points : landmark.second) {
                Seraphim::Types::Point2D *point = points->add_points();
                point->set_x(landmark_points.x);
                point->set_y(landmark_points.y);
            }
            facemarks_->add_landmarks(type);
        }
    }

    return true;
}
