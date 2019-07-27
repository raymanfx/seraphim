/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <utils.h>

#include "facemark_detector_service.h"

using namespace sph::face;

FacemarkDetectorService::FacemarkDetectorService(
    std::shared_ptr<sph::face::IFaceDetector> face_detector,
    std::shared_ptr<sph::face::IFacemarkDetector> facemark_detector) {
    m_face_detector = face_detector;
    m_facemark_detector = facemark_detector;
}

bool FacemarkDetectorService::handle_request(const Seraphim::Request &req,
                                             Seraphim::Response &res) {
    if (!req.has_face() || !req.face().has_facemark_detector()) {
        return false;
    }

    if (req.face().facemark_detector().has_detection()) {
        return handle_detection_request(
            req.face().facemark_detector().detection(),
            *res.mutable_face()->mutable_facemark_detector()->mutable_detection());
    }

    return false;
}

bool FacemarkDetectorService::handle_detection_request(
    const Seraphim::Face::FacemarkDetector::DetectionRequest &req,
    Seraphim::Face::FacemarkDetector::DetectionResponse &res) {
    cv::Mat image;
    std::vector<cv::Rect> faces;
    std::vector<sph::face::IFacemarkDetector::Facemarks> facemarks;
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

    m_face_detector->detect_faces(image(roi), faces);
    m_facemark_detector->detect_facemarks(image(roi), faces, facemarks);

    for (size_t i = 0; i < faces.size(); i++) {
        Seraphim::Types::Region2D *face = res.add_faces();
        face->set_x(faces[i].x);
        face->set_y(faces[i].y);
        face->set_w(faces[i].width);
        face->set_h(faces[i].height);
    }

    for (const auto &face : facemarks) {
        for (const auto &landmark : face.landmarks) {
            Seraphim::Face::FacemarkDetector::Facemarks::Landmark type;
            switch (landmark.first) {
            case IFacemarkDetector::FacemarkType::JAW:
                type = Seraphim::Face::FacemarkDetector::Facemarks::JAW;
                break;
            case IFacemarkDetector::FacemarkType::RIGHT_EYEBROW:
                type = Seraphim::Face::FacemarkDetector::Facemarks::RIGHT_EYEBROW;
                break;
            case IFacemarkDetector::FacemarkType::LEFT_EYEBROW:
                type = Seraphim::Face::FacemarkDetector::Facemarks::LEFT_EYEBROW;
                break;
            case IFacemarkDetector::FacemarkType::NOSE:
                type = Seraphim::Face::FacemarkDetector::Facemarks::NOSE;
                break;
            case IFacemarkDetector::FacemarkType::RIGHT_EYE:
                type = Seraphim::Face::FacemarkDetector::Facemarks::RIGHT_EYE;
                break;
            case IFacemarkDetector::FacemarkType::LEFT_EYE:
                type = Seraphim::Face::FacemarkDetector::Facemarks::LEFT_EYE;
                break;
            case IFacemarkDetector::FacemarkType::MOUTH:
                type = Seraphim::Face::FacemarkDetector::Facemarks::MOUTH;
                break;
            default:
                // unknown facemark
                continue;
            }

            Seraphim::Face::FacemarkDetector::Facemarks *facemarks_ = res.add_facemarks();
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