/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <utils.h>

#include "face_detector_service.h"

using namespace sph::face;

FaceDetectorService::FaceDetectorService(std::shared_ptr<sph::face::IFaceDetector> detector) {
    m_detector = detector;
}

bool FaceDetectorService::handle_request(const Seraphim::Request &req, Seraphim::Response &res) {
    if (!req.has_face() || !req.face().has_face_detector()) {
        return false;
    }

    if (req.face().face_detector().has_detection()) {
        return handle_detection_request(
            req.face().face_detector().detection(),
            *res.mutable_face()->mutable_face_detector()->mutable_detection());
    }

    return false;
}

bool FaceDetectorService::handle_detection_request(
    const Seraphim::Face::FaceDetector::DetectionRequest &req,
    Seraphim::Face::FaceDetector::DetectionResponse &res) {
    cv::Mat image;
    std::vector<cv::Rect> faces;
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

    for (size_t i = 0; i < faces.size(); i++) {
        Seraphim::Types::Region2D *face = res.add_faces();
        face->set_x(faces[i].x);
        face->set_y(faces[i].y);
        face->set_w(faces[i].width);
        face->set_h(faces[i].height);
    }

    return true;
}
