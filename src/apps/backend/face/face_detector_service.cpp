/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <seraphim/core/polygon.h>
#include <seraphim/iop/opencv/mat.h>
#include <utils.h>

#include "face_detector_service.h"

using namespace sph::core;
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
    Image image;
    std::vector<Polygon<int>> faces;
    cv::Mat mat;
    cv::Rect2i roi;

    if (!sph::backend::Image2DtoMat(req.image(), mat)) {
        return false;
    }

    roi = cv::Rect2i(0, 0, mat.cols, mat.rows);

    if (req.has_roi()) {
        roi.x = req.roi().x();
        roi.y = req.roi().y();
        roi.width = req.roi().w();
        roi.height = req.roi().h();
    }

    mat = mat(roi);
    image = sph::iop::cv::MatFacility::to_image(mat);
    if (image.empty()) {
        return false;
    }

    m_detector->detect_faces(image, faces);

    for (const auto &poly : faces) {
        Seraphim::Types::Region2D *face = res.add_faces();
        face->set_x(poly.bl().x);
        face->set_y(poly.bl().y);
        face->set_w(poly.width());
        face->set_h(poly.height());
    }

    return true;
}
