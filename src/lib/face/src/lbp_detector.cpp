/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include "seraphim/face/lbp_detector.h"

using namespace sph::face;

static bool face_cascade_impl_helper(cv::InputArray img, cv::OutputArray ROIs, void *data) {
    return reinterpret_cast<LBPDetector *>(data)->face_cascade_impl(img, ROIs);
}

LBPDetector::LBPDetector() {
    m_impl = cv::face::FacemarkLBF::create(m_facemark_params);
    m_face_cascade = nullptr;
    m_facemark_model_loaded = false;

    m_params.cascade_scale_factor = 1.1;
    m_params.cascade_min_neighbours = 3;
    m_params.cascade_flags = cv::CASCADE_SCALE_IMAGE;
    m_params.cascade_min_size = cv::Size(30, 30);

    m_target = TARGET_CPU;
}

LBPDetector::~LBPDetector() {
    m_impl.release();
}

bool LBPDetector::face_cascade_impl(cv::InputArray img, cv::OutputArray ROIs) {
    cv::Mat gray;
    cv::UMat gray_umat;
    std::vector<cv::Rect> faces;

    // preprocessing stage: create the appropriate input buffer
    // convert to 8-bit single channel if necessary
    switch (m_target) {
    case TARGET_CPU:
        if (img.channels() > 1) {
            cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
            cv::equalizeHist(gray, gray);
        } else {
            cv::equalizeHist(img, gray);
        }
        break;
    case TARGET_OPENCL:
        if (img.channels() > 1) {
            cv::cvtColor(img, gray_umat, cv::COLOR_BGR2GRAY);
            cv::equalizeHist(gray_umat, gray_umat);
        } else {
            cv::equalizeHist(img, gray_umat);
        }
        break;
    default:
        /* unsupported */
        return false;
    }

    // main stage: run the cascade classifier to detect faces
    switch (m_target) {
    case TARGET_CPU:
        m_face_cascade->detectMultiScale(gray, faces, m_params.cascade_scale_factor,
                                         m_params.cascade_min_neighbours, m_params.cascade_flags,
                                         m_params.cascade_min_size);
        break;
    case TARGET_OPENCL:
        m_face_cascade->detectMultiScale(gray_umat, faces, m_params.cascade_scale_factor,
                                         m_params.cascade_min_neighbours, m_params.cascade_flags,
                                         m_params.cascade_min_size);
        break;
    default:
        /* unsupported */
        return false;
    }

    cv::Mat(faces).copyTo(ROIs);
    return true;
};

bool LBPDetector::load_face_cascade(const std::string &path) {
    m_facemark_model_loaded = false;

    if (path.empty()) {
        return false;
    }

    m_face_cascade = std::unique_ptr<cv::CascadeClassifier>(new cv::CascadeClassifier(path));
    m_impl->setFaceDetector(face_cascade_impl_helper, this);
    return true;
}

bool LBPDetector::load_facemark_model(const std::string &path) {
    m_facemark_model_loaded = false;

    if (path.empty()) {
        return false;
    }

    m_impl->loadModel(path);
    m_facemark_model_loaded = true;
    return true;
}

bool LBPDetector::detect_faces(cv::InputArray img, cv::OutputArray ROIs) {
    if (!m_face_cascade || m_impl.empty()) {
        return false;
    }

    return m_impl->getFaces(img, ROIs);
}

bool LBPDetector::detect_facemarks(cv::InputArray img, cv::InputArray faces,
                                   cv::OutputArrayOfArrays facemarks) {
    if (!m_face_cascade || m_impl.empty()) {
        return false;
    }

    if (!m_facemark_model_loaded) {
        std::cout << "[ERROR] face_detector::" << __func__ << ": Facemark model not loaded"
                  << std::endl;
        return false;
    }

    return m_impl->fit(img, faces, facemarks);
}

bool LBPDetector::find_eyes(const std::vector<cv::Point2f> &facemarks,
                            std::vector<cv::Point2f> &eyes) const {
    cv::Point2f leftEye, rightEye;
    std::vector<int> leftEyemarks, rightEyemarks;

    eyes.clear();

    // compute centers of the eyes
    for (size_t i = 0; i < m_facemark_params.pupils[0].size(); i++) {
        leftEyemarks.push_back(m_facemark_params.pupils[0][i]);
    }
    for (size_t i = 0; i < m_facemark_params.pupils[1].size(); i++) {
        rightEyemarks.push_back(m_facemark_params.pupils[1][i]);
    }

    if (leftEyemarks.size() == 0 || rightEyemarks.size() == 0) {
        // could not find the indices
        return false;
    }

    // left and right eye points in the image
    for (const auto &idx : leftEyemarks) {
        leftEye.x += facemarks[static_cast<size_t>(idx)].x;
        leftEye.y += facemarks[static_cast<size_t>(idx)].y;
    }
    leftEye.x /= leftEyemarks.size();
    leftEye.y /= leftEyemarks.size();

    for (const auto &idx : rightEyemarks) {
        rightEye.x += facemarks[static_cast<size_t>(idx)].x;
        rightEye.y += facemarks[static_cast<size_t>(idx)].y;
    }
    rightEye.x /= leftEyemarks.size();
    rightEye.y /= leftEyemarks.size();

    eyes.push_back(leftEye);
    eyes.push_back(rightEye);
    return true;
}

bool LBPDetector::set_target(const target_t &target) {
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
