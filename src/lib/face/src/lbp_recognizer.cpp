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

void LBPRecognizer::update(cv::InputArrayOfArrays imgs, const std::vector<int> &labels,
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
        m_faces[labels[i]].push_back(images[i]);
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

bool LBPRecognizer::predict(cv::InputArray img, std::vector<Prediction> &preds) {
    Prediction pred = {};
    cv::Mat gray;
    cv::UMat gray_umat;

    // respect input mat type: separate codepaths for mat/umat
    if (img.isUMat()) {
        // convert to 8-bit single channel if necessary
        if (img.channels() > 1) {
            cv::cvtColor(img, gray_umat, cv::COLOR_BGR2GRAY);
        } else {
            gray_umat = img.getUMat(cv::ACCESS_READ);
        }
    } else {
        if (img.channels() > 1) {
            cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = img.getMat(cv::ACCESS_READ);
        }
    }

    preds.clear();
    pred.rect = cv::Rect();

    if (m_faces.size() == 0) {
        return false;
    }

    if (img.isUMat()) {
        m_impl->predict(gray_umat, pred.label, pred.confidence);
    } else {
        m_impl->predict(gray, pred.label, pred.confidence);
    }

    if (pred.label != -1) {
        preds.push_back(pred);
    }

    return true;
}
