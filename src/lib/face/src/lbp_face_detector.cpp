/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include "seraphim/face/lbp_face_detector.h"

using namespace sph::face;

LBPFaceDetector::LBPFaceDetector() {
    m_params.cascade_scale_factor = 1.1;
    m_params.cascade_min_neighbours = 3;
    m_params.cascade_flags = cv::CASCADE_SCALE_IMAGE;
    m_params.cascade_min_size = cv::Size(30, 30);

    m_target = TARGET_CPU;
}

LBPFaceDetector::~LBPFaceDetector() {
    // dummy
}

bool LBPFaceDetector::face_cascade_impl(cv::InputArray img, cv::OutputArray ROIs) {
    cv::Mat gray;
    cv::UMat gray_umat;
    std::vector<cv::Rect> faces;
    std::unique_lock<std::mutex> lock(m_target_mutex);

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
        m_face_cascade.detectMultiScale(gray, faces, m_params.cascade_scale_factor,
                                        m_params.cascade_min_neighbours, m_params.cascade_flags,
                                        m_params.cascade_min_size);
        break;
    case TARGET_OPENCL:
        m_face_cascade.detectMultiScale(gray_umat, faces, m_params.cascade_scale_factor,
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

bool LBPFaceDetector::load_face_cascade(const std::string &path) {
    if (path.empty()) {
        return false;
    }

    return m_face_cascade.load(path);
}

bool LBPFaceDetector::detect_faces(cv::InputArray img, cv::OutputArray ROIs) {
    if (m_face_cascade.empty()) {
        return false;
    }

    return face_cascade_impl(img, ROIs);
}

bool LBPFaceDetector::set_target(const target_t &target) {
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