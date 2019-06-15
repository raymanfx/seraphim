/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <utils.h>

#include "classifier_service.h"

using namespace sph::object;

ClassifierService::ClassifierService(sph::object::Classifier *recognizer) {
    m_recognizer = recognizer;
}

bool ClassifierService::handle_request(const Seraphim::Request &req, Seraphim::Response &res) {
    if (!req.has_object() || !req.object().has_classifier()) {
        return false;
    }

    if (req.object().classifier().has_classification()) {
        return handle_classification_request(
            req.object().classifier().classification(),
            *res.mutable_object()->mutable_classifier()->mutable_classification());
    }

    return false;
}

bool ClassifierService::handle_classification_request(
    const Seraphim::Object::Classifier::ClassificationRequest &req,
    Seraphim::Object::Classifier::ClassificationResponse &res) {
    cv::Mat image;
    cv::Rect2i roi;
    std::vector<sph::object::Classifier::Prediction> predictions;

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

    m_recognizer->predict(image(roi), predictions);
    for (size_t i = 0; i < predictions.size(); i++) {
        // filter results if a global threshold is set
        if (req.confidence() > 0.0f && predictions[i].confidence < req.confidence()) {
            continue;
        }

        res.add_labels(predictions[i].class_id);
        res.add_confidences(predictions[i].confidence);
        Seraphim::Types::Region2D *roi = res.add_rois();
        roi->set_x(predictions[i].rect.x);
        roi->set_y(predictions[i].rect.y);
        roi->set_w(predictions[i].rect.width);
        roi->set_h(predictions[i].rect.height);
    }

    return true;
}
