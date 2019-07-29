/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include "seraphim/face/lbf_facemark_detector.h"
#include "seraphim/core/image_utils_opencv.h"

using namespace sph::core;
using namespace sph::face;

static constexpr std::pair<IFacemarkDetector::FacemarkType, std::pair<size_t, size_t>>
    Facemark_LUT[] = { { IFacemarkDetector::FacemarkType::JAW, { 0, 16 } },
                       { IFacemarkDetector::FacemarkType::RIGHT_EYEBROW, { 17, 21 } },
                       { IFacemarkDetector::FacemarkType::LEFT_EYEBROW, { 22, 26 } },
                       { IFacemarkDetector::FacemarkType::NOSE, { 27, 35 } },
                       { IFacemarkDetector::FacemarkType::RIGHT_EYE, { 36, 41 } },
                       { IFacemarkDetector::FacemarkType::LEFT_EYE, { 42, 47 } },
                       { IFacemarkDetector::FacemarkType::MOUTH, { 48, 67 } } };

LBFFacemarkDetector::LBFFacemarkDetector(std::shared_ptr<IFaceDetector> detector) {
    m_detector = detector;
    m_facemark_impl = cv::face::FacemarkLBF::create(m_facemark_params);

    m_target = TARGET_CPU;
}

LBFFacemarkDetector::~LBFFacemarkDetector() {
    m_facemark_impl.release();
}

bool LBFFacemarkDetector::load_facemark_model(const std::string &path) {
    if (path.empty()) {
        return false;
    }

    m_facemark_impl->loadModel(path);
    if (m_facemark_impl.empty()) {
        return false;
    }

    return true;
}

bool LBFFacemarkDetector::detect_facemarks(const sph::core::Image &img,
                                           const std::vector<sph::core::Polygon<>> &faces,
                                           std::vector<Facemarks> &facemarks) {
    std::vector<cv::Rect> faces_;
    std::vector<std::vector<cv::Point2f>> landmarks;
    cv::Mat mat;
    cv::UMat umat;

    if (faces.empty()) {
        return false;
    }

    // prepare the compute buffer
    switch (m_target) {
    case TARGET_CPU:
        if (!Image2Mat(img, mat)) {
            return false;
        }
        break;
    case TARGET_OPENCL:
        if (!Image2Mat(img, mat)) {
            return false;
        }
        mat.copyTo(umat);
        break;
    default:
        return false;
    }

    if (m_facemark_impl.empty()) {
        std::cout << "[ERROR] LBFFaceMarkDetector::" << __func__ << ": model not loaded"
                  << std::endl;
        return false;
    }

    for (const auto &poly : faces) {
        faces_.emplace_back(cv::Rect(poly.bl().x, poly.bl().y, poly.width(), poly.height()));
    }

    // perform the actual detection
    switch (m_target) {
    case TARGET_CPU:
        if (!m_facemark_impl->fit(mat, faces_, landmarks)) {
            return false;
        }
        break;
    case TARGET_OPENCL:
        if (!m_facemark_impl->fit(umat, faces_, landmarks)) {
            return false;
        }
        break;
    default:
        return false;
    }

    facemarks.clear();
    for (const auto &facepoints : landmarks) {
        Facemarks marks;

        for (const auto &elem : Facemark_LUT) {
            std::vector<sph::core::Polygon<>::Point> points;

            for (size_t i = elem.second.first; i <= elem.second.second; i++) {
                points.push_back(
                    { static_cast<int>(facepoints[i].x), static_cast<int>(facepoints[i].y) });
            }
            marks.landmarks.emplace_back(std::make_pair(elem.first, points));
        }
        facemarks.push_back(marks);
    }

    return true;
}

bool LBFFacemarkDetector::set_target(const target_t &target) {
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
