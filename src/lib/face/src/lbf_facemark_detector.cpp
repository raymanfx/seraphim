/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <seraphim/iop/opencv/mat.h>

#include "seraphim/face/lbf_facemark_detector.h"

using namespace sph;
using namespace sph::face;

static constexpr std::pair<IFacemarkDetector::FacemarkType, std::pair<size_t, size_t>>
    Facemark_LUT[] = { { IFacemarkDetector::FacemarkType::JAW, { 0, 16 } },
                       { IFacemarkDetector::FacemarkType::RIGHT_EYEBROW, { 17, 21 } },
                       { IFacemarkDetector::FacemarkType::LEFT_EYEBROW, { 22, 26 } },
                       { IFacemarkDetector::FacemarkType::NOSE, { 27, 35 } },
                       { IFacemarkDetector::FacemarkType::RIGHT_EYE, { 36, 41 } },
                       { IFacemarkDetector::FacemarkType::LEFT_EYE, { 42, 47 } },
                       { IFacemarkDetector::FacemarkType::MOUTH, { 48, 67 } } };

LBFFacemarkDetector::LBFFacemarkDetector() {
    m_facemark_impl = cv::face::FacemarkLBF::create(m_facemark_params);
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

bool LBFFacemarkDetector::detect_facemarks(const sph::Image &img,
                                           const std::vector<sph::Polygon<int>> &faces,
                                           std::vector<Facemarks> &facemarks) {
    std::vector<cv::Rect> faces_;
    std::vector<std::vector<cv::Point2f>> landmarks;
    cv::Mat mat;
    cv::UMat umat;

    if (faces.empty()) {
        return false;
    }

    mat = sph::iop::cv::MatFacility::from_image(img);
    if (mat.empty()) {
        return false;
    }

    // prepare the compute buffer
    switch (m_target) {
    case Target::CPU:
        break;
    case Target::OPENCL:
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
        faces_.emplace_back(cv::Rect(poly.tl().x, poly.tl().y, poly.width(), poly.height()));
    }

    // perform the actual detection
    switch (m_target) {
    case Target::CPU:
        if (!m_facemark_impl->fit(mat, faces_, landmarks)) {
            return false;
        }
        break;
    case Target::OPENCL:
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
            std::vector<sph::Point2i> points;

            for (size_t i = elem.second.first; i <= elem.second.second; i++) {
                points.emplace_back(static_cast<int>(facepoints[i].x),
                                    static_cast<int>(facepoints[i].y));
            }
            marks.landmarks.emplace_back(std::make_pair(elem.first, points));
        }
        facemarks.push_back(marks);
    }

    return true;
}

bool LBFFacemarkDetector::set_target(Target target) {
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
