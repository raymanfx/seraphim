/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include "seraphim/object/dnn_classifier.h"

using namespace sph::object;

DNNClassifier::DNNClassifier() {
    m_refresh_layer_names = false;
}

DNNClassifier::~DNNClassifier() {
    // dummy
}

bool DNNClassifier::read_net(const std::string &model, const std::string &config,
                             const std::string &framework) {
    try {
        m_net = cv::dnn::readNet(model, config, framework);
    } catch (...) {
        return false;
    }

    m_refresh_layer_names = true;
    return true;
}

std::vector<std::string> DNNClassifier::get_unconnected_out_layer_names() {
    static std::vector<std::string> names;

    if (m_refresh_layer_names || names.empty()) {
        m_refresh_layer_names = false;
        std::vector<int> out_layers = m_net.getUnconnectedOutLayers();
        std::vector<cv::String> layer_names = m_net.getLayerNames();
        names.resize(out_layers.size());
        for (size_t i = 0; i < out_layers.size(); ++i) {
            names[i] = layer_names[static_cast<size_t>(out_layers[i] - 1)];
        }
    }

    return names;
}

bool DNNClassifier::set_target(const target_t &target) {
    std::unique_lock<std::mutex> lock(m_target_mutex);

    switch (target) {
    case TARGET_CPU:
        m_net.setPreferableTarget(cv::dnn::Target::DNN_TARGET_CPU);
        break;
    case TARGET_OPENCL:
        m_net.setPreferableTarget(cv::dnn::Target::DNN_TARGET_OPENCL);
        break;
    case TARGET_OPENCL_FP16:
        m_net.setPreferableTarget(cv::dnn::Target::DNN_TARGET_OPENCL_FP16);
        break;
    default:
        /* unsupported */
        return false;
    }

    return true;
}

bool DNNClassifier::predict(cv::InputArray img, std::vector<Prediction> &preds) {
    cv::Mat blob;
    std::vector<cv::Mat> outputs;
    std::vector<int> out_layers;
    std::vector<cv::String> out_layer_names;
    std::vector<std::string> out_layer_types;
    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    std::unique_lock<std::mutex> lock(m_target_mutex);

    preds.clear();

    // create a 4D blob and feed it to the net
    // in most common cases, RGB images are used for training, but OpenCV always stores images in
    // BGR order, so we set swapRB to "true" by default
    cv::dnn::blobFromImage(img, blob, 1.0, cv::Size(300, 300), cv::Scalar(), true /* swapRB */);
    m_net.setInput(blob);

    // infer
    out_layer_names = m_net.getUnconnectedOutLayersNames();
    m_net.forward(outputs, out_layer_names);

    // get the output layer type
    //  MobileNet SSDv2: "DetectionOutput"
    //  YOLOv3: "Region"
    out_layers = m_net.getUnconnectedOutLayers();
    out_layer_types.push_back(m_net.getLayer(out_layers[0])->type);

    if (out_layer_types[0] == "DetectionOutput") {
        // Network produces output blob with a shape 1x1xNx7 where N is a number
        // of detections and an every detection is a vector of values [batchId,
        // classId, confidence, left, top, right, bottom]
        for (size_t k = 0; k < outputs.size(); k++) {
            float *data = reinterpret_cast<float *>(outputs[k].data);
            for (size_t i = 0; i < outputs[k].total(); i += 7) {
                float confidence = data[i + 2];
                int left = static_cast<int>(data[i + 3]);
                int top = static_cast<int>(data[i + 4]);
                int right = static_cast<int>(data[i + 5]);
                int bottom = static_cast<int>(data[i + 6]);
                int width = right - left + 1;
                int height = bottom - top + 1;
                if (width * height <= 1) {
                    left = static_cast<int>(data[i + 3] * img.cols());
                    top = static_cast<int>(data[i + 4] * img.rows());
                    right = static_cast<int>(data[i + 5] * img.cols());
                    bottom = static_cast<int>(data[i + 6] * img.rows());
                    width = right - left + 1;
                    height = bottom - top + 1;
                }

                class_ids.push_back(static_cast<int>(data[i + 1]));
                confidences.push_back(confidence);
                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
    } else if (out_layer_types[0] == "Region") {
        for (size_t i = 0; i < outputs.size(); ++i) {
            // Network produces output blob with a shape NxC where N is a number of
            // detected objects and C is a number of classes + 4 where the first 4
            // numbers are [center_x, center_y, width, height]
            float *data = reinterpret_cast<float *>(outputs[i].data);
            for (int j = 0; j < outputs[i].rows; ++j, data += outputs[i].cols) {
                cv::Mat scores = outputs[i].row(j).colRange(5, outputs[i].cols);
                cv::Point class_id_point;
                double confidence;
                minMaxLoc(scores, nullptr, &confidence, nullptr, &class_id_point);
                int centerX = static_cast<int>(data[0] * img.cols());
                int centerY = static_cast<int>(data[1] * img.rows());
                int width = static_cast<int>(data[2] * img.cols());
                int height = static_cast<int>(data[3] * img.rows());
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                class_ids.push_back(class_id_point.x);
                confidences.push_back(static_cast<float>(confidence));
                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
    } else {
        return false;
    }

    // do non maximum suppression (NMS) to filter out objects suppressed by bigger ones
    std::vector<int> nms_indices;
    cv::dnn::NMSBoxes(boxes, confidences, 0.0, 0.4f, nms_indices);

    for (size_t i = 0; i < nms_indices.size(); i++) {
        size_t idx = static_cast<size_t>(nms_indices[i]);
        Prediction pred = {};
        pred.class_id = class_ids[idx];
        pred.confidence = confidences[idx];
        pred.rect = boxes[idx];
        preds.push_back(pred);
    }

    return true;
}
