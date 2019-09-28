/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <seraphim/iop/opencv/mat.h>

#include "seraphim/object/dnn_classifier.h"

using namespace sph::core;
using namespace sph::object;

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

bool DNNClassifier::set_target(Target target) {
    std::unique_lock<std::mutex> lock(m_target_mutex);

    switch (target) {
    case Target::CPU:
        m_net.setPreferableTarget(cv::dnn::Target::DNN_TARGET_CPU);
        break;
    case Target::OPENCL:
        m_net.setPreferableTarget(cv::dnn::Target::DNN_TARGET_OPENCL);
        break;
    case Target::OPENCL_FP16:
        m_net.setPreferableTarget(cv::dnn::Target::DNN_TARGET_OPENCL_FP16);
        break;
#ifdef WITH_VULKAN
    case Target::VULKAN:
        m_net.setPreferableBackend(cv::dnn::Backend::DNN_BACKEND_VKCOM);
        m_net.setPreferableTarget(cv::dnn::Target::DNN_TARGET_VULKAN);
        break;
#endif
    default:
        /* unsupported */
        return false;
    }

    return true;
}

bool DNNClassifier::predict(const Image &img, std::vector<Prediction> &preds) {
    cv::Mat mat;
    cv::Mat blob;
    std::vector<cv::Mat> outputs;
    std::vector<int> out_layers;
    std::vector<cv::String> out_layer_names;
    std::vector<std::string> out_layer_types;
    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    std::unique_lock<std::mutex> lock(m_target_mutex);

    mat = sph::iop::cv::MatFacility::from_image(img);
    if (mat.empty()) {
        return false;
    }

    preds.clear();

    // create a 4D blob and feed it to the net
    // in most common cases, RGB images are used for training, but OpenCV always stores images in
    // BGR order, so we set swapRB to "true" by default
    cv::dnn::blobFromImage(mat, blob, m_blob_params.scalefactor, m_blob_params.size,
                           m_blob_params.mean, m_blob_params.swap_rb, m_blob_params.crop);
    m_net.setInput(blob);

    // infer
    out_layer_names = m_net.getUnconnectedOutLayersNames();
    m_net.forward(outputs, out_layer_names);

    // get the output layer type
    //  MobileNet SSDv2: "DetectionOutput"
    //  YOLOv3: "Region"
    //  Caffe: "DetectionOutput"
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
                if (width * height <= 4) {
                    left = static_cast<int>(data[i + 3] * mat.cols);
                    top = static_cast<int>(data[i + 4] * mat.rows);
                    right = static_cast<int>(data[i + 5] * mat.cols);
                    bottom = static_cast<int>(data[i + 6] * mat.rows);
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
                int centerX = static_cast<int>(data[0] * mat.cols);
                int centerY = static_cast<int>(data[1] * mat.rows);
                int width = static_cast<int>(data[2] * mat.cols);
                int height = static_cast<int>(data[3] * mat.rows);
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
        pred.poly = Polygon<int>(
            Point2i(boxes[idx].x, boxes[idx].y),
            Point2i(boxes[idx].x, boxes[idx].y + boxes[idx].height),
            Point2i(boxes[idx].x + boxes[idx].width, boxes[idx].y),
            Point2i(boxes[idx].x + boxes[idx].width, boxes[idx].y + boxes[idx].height));
        preds.push_back(pred);
    }

    return true;
}
