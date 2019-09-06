/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <seraphim/iop/opencv/mat.h>

#include "seraphim/face/lbp_face_detector.h"

using namespace sph::core;
using namespace sph::face;

bool LBPFaceDetector::face_cascade_impl(cv::InputArray img, cv::OutputArray ROIs) {
    cv::Mat gray;
    cv::UMat gray_umat;
    std::vector<cv::Rect> faces;
    std::unique_lock<std::mutex> lock(m_target_mutex);

    // preprocessing stage: create the appropriate input buffer
    // convert to 8-bit single channel if necessary
    switch (m_target) {
    case Target::CPU:
        if (img.channels() > 1) {
            cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
            cv::equalizeHist(gray, gray);
        } else {
            cv::equalizeHist(img, gray);
        }
        break;
    case Target::OPENCL:
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
    case Target::CPU:
        m_face_cascade.detectMultiScale(gray, faces, m_params.cascade_scale_factor,
                                        m_params.cascade_min_neighbours, m_params.cascade_flags,
                                        m_params.cascade_min_size);
        break;
    case Target::OPENCL:
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

bool LBPFaceDetector::detect_faces(const Image &img, std::vector<Polygon<int>> &faces) {
    cv::Mat mat;
    std::vector<cv::Rect> faces_;

    if (m_face_cascade.empty()) {
        return false;
    }

    mat = sph::iop::cv::MatFacility::from_image(img);
    cv::cvtColor(mat, mat, cv::COLOR_BGR2GRAY);
    if (mat.empty()) {
        return false;
    }

    if (!face_cascade_impl(mat, faces_)) {
        return false;
    }

    faces.clear();
    for (const auto &rect : faces_) {
        faces.emplace_back(Polygon<int>(Point2i(rect.x, rect.y),
                                        Point2i(rect.x, rect.y + rect.height),
                                        Point2i(rect.x + rect.width, rect.y),
                                        Point2i(rect.x + rect.width, rect.y + rect.height)));
    }

    return true;
}

bool LBPFaceDetector::set_target(const Target &target) {
    std::unique_lock<std::mutex> lock(m_target_mutex);

    switch (target) {
    case Target::CPU:
    case Target::OPENCL:
        m_target = target;
        break;
    default:
        /* unsupported */
        return false;
    }

    return true;
}
