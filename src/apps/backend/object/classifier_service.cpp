/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <seraphim/iop/opencv/mat.h>
#include <utils.h>

#include "classifier_service.h"

using namespace sph::object;

ClassifierService::ClassifierService(std::shared_ptr<sph::object::Classifier> recognizer) {
    m_recognizer = recognizer;
}

bool ClassifierService::handle_request(const Seraphim::Request &req, Seraphim::Response &res) {
    if (req.inner().Is<Seraphim::Object::Detector::DetectionRequest>()) {
        Seraphim::Object::Detector::DetectionRequest inner_req;
        Seraphim::Object::Detector::DetectionResponse inner_res;

        req.inner().UnpackTo(&inner_req);
        if (handle_detection_request(inner_req, inner_res)) {
            res.mutable_inner()->PackFrom(inner_res);
            return true;
        }
    }

    return false;
}

bool ClassifierService::handle_detection_request(
    const Seraphim::Object::Detector::DetectionRequest &req,
    Seraphim::Object::Detector::DetectionResponse &res) {
    sph::CoreImage image;
    cv::Mat mat;
    cv::Rect2i roi;
    std::vector<sph::object::Classifier::Prediction> predictions;

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

    m_recognizer->predict(image, predictions);
    for (size_t i = 0; i < predictions.size(); i++) {
        // filter results if a global threshold is set
        if (req.confidence() > 0.0f && predictions[i].confidence < req.confidence()) {
            continue;
        }

        res.add_labels(predictions[i].class_id);
        res.add_confidences(predictions[i].confidence);
        Seraphim::Types::Region2D *roi = res.add_rois();
        roi->set_x(predictions[i].poly.bl().x);
        roi->set_y(predictions[i].poly.bl().y);
        roi->set_w(predictions[i].poly.width());
        roi->set_h(predictions[i].poly.height());
    }

    return true;
}
