/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include "seraphim/face/lbp_detector.h"

using namespace sph::face;

static constexpr std::pair<IDetector::FacemarkType, std::pair<size_t, size_t>> Facemark_LUT[] = {
    { IDetector::FacemarkType::JAW,             { 0, 16 } },
    { IDetector::FacemarkType::RIGHT_EYEBROW,   { 17, 21 } },
    { IDetector::FacemarkType::LEFT_EYEBROW,    { 22, 26 } },
    { IDetector::FacemarkType::NOSE,            { 27, 35 } },
    { IDetector::FacemarkType::RIGHT_EYE,       { 36, 41 } },
    { IDetector::FacemarkType::LEFT_EYE,        { 42, 47 } },
    { IDetector::FacemarkType::MOUTH,           { 48, 67 } }
};

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
                                   std::vector<Facemarks> &facemarks) {
    std::vector<std::vector<cv::Point2f>> landmarks;

    if (!m_face_cascade || m_impl.empty()) {
        return false;
    }

    if (!m_facemark_model_loaded) {
        std::cout << "[ERROR] face_detector::" << __func__ << ": Facemark model not loaded"
                  << std::endl;
        return false;
    }

    if (!m_impl->fit(img, faces, landmarks)) {
        return false;
    }

    facemarks.clear();
    for (const auto &facepoints : landmarks) {
        Facemarks marks;

        for (const auto &elem : Facemark_LUT) {
            std::vector<cv::Point> points;

            for (size_t i = elem.second.first; i <= elem.second.second; i++) {
                points.push_back(facepoints[i]);
            }

            marks.landmarks.emplace_back(std::make_pair(elem.first, points));
        }

        facemarks.push_back(marks);
    }

    return true;
}

bool LBPDetector::set_target(const target_t &target) {
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
