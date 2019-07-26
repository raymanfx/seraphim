/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include "seraphim/face/lbp_recognizer.h"

using namespace sph::face;

LBPRecognizer::LBPRecognizer() {
    m_impl = cv::face::LBPHFaceRecognizer::create();
}

LBPRecognizer::~LBPRecognizer() {
    // dummy
}

void LBPRecognizer::train(cv::InputArrayOfArrays imgs, const std::vector<int> &labels) {
    // invalidate old faces
    m_faces.clear();

    // deep copy of new faces
    for (size_t i = 0; i < labels.size(); i++) {
        imgs.getMatVector(m_faces[labels[i]]);
    }

    m_impl->train(imgs, labels);
}

void LBPRecognizer::update(cv::InputArrayOfArrays imgs, const std::vector<int> &labels,
                           bool invalidate) {
    const std::vector<cv::Mat> &images_ =
        *(reinterpret_cast<const std::vector<cv::Mat> *>(imgs.getObj()));

    if (invalidate) {
        // invalidate face images for the new labels
        for (size_t i = 0; i < labels.size(); i++) {
            m_faces.erase(labels[i]);
        }
    }

    for (size_t i = 0; i < labels.size(); i++) {
        // add the new face images
        m_faces[labels[i]].push_back(images_[i]);
    }

    if (!invalidate) {
        // directly use the parameters
        m_impl->update(imgs, labels);
    } else {
        // re-train
        m_impl->train(imgs, labels);
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

bool LBPRecognizer::predict(cv::InputArray img, std::vector<Prediction> &preds) {
    Prediction pred = {};

    preds.clear();
    pred.rect = cv::Rect();

    if (m_faces.size() == 0) {
        return false;
    }

    m_impl->predict(img, pred.label, pred.confidence);
    if (pred.label != -1) {
        preds.push_back(pred);
    }

    return true;
}
