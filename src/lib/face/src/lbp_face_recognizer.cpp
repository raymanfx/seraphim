/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include "seraphim/face/lbp_face_recognizer.h"

using namespace sph::face;

LBPFaceRecognizer::LBPFaceRecognizer() {
    m_impl = cv::face::LBPHFaceRecognizer::create();

    m_target = TARGET_CPU;
}

LBPFaceRecognizer::~LBPFaceRecognizer() {
    // dummy
}

void LBPFaceRecognizer::train(cv::InputArrayOfArrays imgs, const std::vector<int> &labels) {
    std::vector<cv::Mat> gray_imgs;

    // invalidate old faces
    m_faces.clear();

    // deep copy of new faces
    for (size_t i = 0; i < labels.size(); i++) {
        imgs.getMatVector(m_faces[labels[i]]);
    }

    // convert to 8-bit single channel if necessary
    for (const auto &person : m_faces) {
        for (const auto &face_img : person.second) {
            gray_imgs.push_back(cv::Mat());
            if (face_img.channels() > 1) {
                cv::cvtColor(face_img, gray_imgs[gray_imgs.size() - 1], cv::COLOR_BGR2GRAY);
            } else {
                gray_imgs[gray_imgs.size() - 1] = face_img;
            }
        }
    }

    m_impl->train(gray_imgs, labels);
}

void LBPFaceRecognizer::update(cv::InputArrayOfArrays imgs, const std::vector<int> &labels,
                               bool invalidate) {
    std::vector<cv::Mat> gray_imgs;
    std::vector<cv::Mat> images;

    // convert to 8-bit single channel if necessary
    imgs.getMatVector(images);
    for (const auto &face_img : images) {
        gray_imgs.push_back(cv::Mat());
        if (face_img.channels() > 1) {
            cv::cvtColor(face_img, gray_imgs[gray_imgs.size() - 1], cv::COLOR_BGR2GRAY);
        } else {
            gray_imgs[gray_imgs.size() - 1] = face_img;
        }
    }

    if (invalidate) {
        // invalidate face images for the new labels
        for (size_t i = 0; i < labels.size(); i++) {
            m_faces.erase(labels[i]);
        }
    }

    for (size_t i = 0; i < labels.size(); i++) {
        // add the new face images
        m_faces[labels[i]].push_back(gray_imgs[i]);
    }

    if (!invalidate) {
        // directly use the parameters
        m_impl->update(gray_imgs, labels);
    } else {
        // re-train
        m_impl->train(gray_imgs, labels);
        // add back faces from our internal database
        for (auto it = m_faces.begin(); it != m_faces.end(); it++) {
            if (std::find(labels.begin(), labels.end(), it->first) != labels.end()) {
                continue;
            }

            std::vector<int> tmpLabels;
            for (size_t j = 0; j < it->second.size(); j++) {
                tmpLabels.push_back(it->first);
            }
            m_impl->update(m_faces[it->first], tmpLabels);
        }
    }
}

bool LBPFaceRecognizer::predict(cv::InputArray img, std::vector<Prediction> &preds) {
    Prediction pred = {};
    cv::Mat gray;
    cv::UMat gray_umat;
    std::unique_lock<std::mutex> lock(m_target_mutex);

    // preprocessing stage: create the appropriate input buffer
    // convert to 8-bit single channel if necessary
    switch (m_target) {
    case TARGET_CPU:
        if (img.channels() > 1) {
            cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = img.getMat(cv::ACCESS_READ);
        }
        break;
    case TARGET_OPENCL:
        if (img.channels() > 1) {
            cv::cvtColor(img, gray_umat, cv::COLOR_BGR2GRAY);
        } else {
            gray_umat = img.getUMat(cv::ACCESS_READ | cv::ACCESS_FAST);
        }
        break;
    default:
        /* unsupported */
        return false;
    }

    preds.clear();
    pred.rect = cv::Rect();

    if (m_faces.size() == 0) {
        return false;
    }

    // main stage: predict the faces using local binary pattern matching
    switch (m_target) {
    case TARGET_CPU:
        m_impl->predict(gray, pred.label, pred.confidence);
        break;
    case TARGET_OPENCL:
        m_impl->predict(gray_umat, pred.label, pred.confidence);
        break;
    default:
        /* unsupported */
        return false;
    }

    if (pred.label != -1) {
        preds.push_back(pred);
    }

    return true;
}

bool LBPFaceRecognizer::set_target(const target_t &target) {
    std::unique_lock<std::mutex> lock(m_target_mutex);

    switch (target) {
    case TARGET_CPU:
    case TARGET_OPENCL:
        m_target = target;
        break;
    default:
        /* unsupported */
        return false;
    }

    return true;
}
