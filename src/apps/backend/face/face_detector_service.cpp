/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <seraphim/iop/opencv/mat.h>
#include <seraphim/polygon.h>
#include <utils.h>

#include "face_detector_service.h"

using namespace sph;
using namespace sph::face;

FaceDetectorService::FaceDetectorService(std::shared_ptr<sph::face::FaceDetector> detector) {
    m_detector = detector;
}

bool FaceDetectorService::handle_request(const Seraphim::Request &req, Seraphim::Response &res) {
    if (req.inner().Is<Seraphim::Face::FaceDetector::DetectionRequest>()) {
        Seraphim::Face::FaceDetector::DetectionRequest inner_req;
        Seraphim::Face::FaceDetector::DetectionResponse inner_res;

        req.inner().UnpackTo(&inner_req);
        if (handle_detection_request(inner_req, inner_res)) {
            res.mutable_inner()->PackFrom(inner_res);
            return true;
        }
    }

    return false;
}

bool FaceDetectorService::handle_detection_request(
    const Seraphim::Face::FaceDetector::DetectionRequest &req,
    Seraphim::Face::FaceDetector::DetectionResponse &res) {
    CoreImage image;
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

    m_detector->detect(image, faces);

    for (const auto &poly : faces) {
        Seraphim::Types::Region2D *face = res.add_faces();
        face->set_x(poly.bl().x);
        face->set_y(poly.bl().y);
        face->set_w(poly.width());
        face->set_h(poly.height());
    }

    return true;
}
