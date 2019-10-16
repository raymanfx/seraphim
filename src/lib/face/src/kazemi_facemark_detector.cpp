/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <dlib/opencv/cv_image.h>
#include <seraphim/iop/opencv/mat.h>

#include "seraphim/face/kazemi_facemark_detector.h"

using namespace sph;
using namespace sph::face;

static constexpr std::pair<FacemarkDetector::FacemarkType, std::pair<size_t, size_t>>
    Facemark_LUT[] = { { FacemarkDetector::FacemarkType::JAW, { 0, 16 } },
                       { FacemarkDetector::FacemarkType::RIGHT_EYEBROW, { 17, 21 } },
                       { FacemarkDetector::FacemarkType::LEFT_EYEBROW, { 22, 26 } },
                       { FacemarkDetector::FacemarkType::NOSE, { 27, 35 } },
                       { FacemarkDetector::FacemarkType::RIGHT_EYE, { 36, 41 } },
                       { FacemarkDetector::FacemarkType::LEFT_EYE, { 42, 47 } },
                       { FacemarkDetector::FacemarkType::MOUTH, { 48, 67 } } };

bool KazemiFacemarkDetector::load_facemark_model(const std::string &path) {
    try {
        dlib::deserialize(path) >> m_predictor;
    } catch (dlib::serialization_error) {
        return false;
    }

    return true;
}

bool KazemiFacemarkDetector::detect_facemarks(const sph::Image &img,
                                              const std::vector<sph::Polygon<int>> &faces,
                                              std::vector<Facemarks> &facemarks) {
    // http://dlib.net/face_landmark_detection_ex.cpp.html
    dlib::array2d<unsigned char> dlib_gray_image;
    std::vector<dlib::rectangle> faces_;
    cv::Mat mat;

    mat = sph::iop::cv::MatFacility::from_image(img);
    if (mat.empty()) {
        return false;
    }

    // convert from cv to dlib image
    switch (mat.channels()) {
    case 1:
        dlib::assign_image(dlib_gray_image, dlib::cv_image<unsigned char>(mat));
        break;
    case 3:
        dlib::assign_image(dlib_gray_image, dlib::cv_image<dlib::bgr_pixel>(mat));
        break;
    default:
        return false;
    }

    // convert faces to dlib rectangles
    for (const auto &face : faces) {
        faces_.emplace_back(
            face.bounding_rect().tl().x, static_cast<unsigned long>(face.bounding_rect().tl().y),
            face.bounding_rect().br().x, static_cast<unsigned long>(face.bounding_rect().br().y));
    }

    // clear the output vector
    facemarks.clear();

    // Now we will go ask the shape_predictor to tell us the pose of
    // each face we detected.
    Facemarks marks;
    std::vector<Point2i> nose_points;
    std::vector<Point2i> right_eye_points;
    std::vector<Point2i> left_eye_points;
    for (const auto &face : faces_) {
        dlib::full_object_detection shape = m_predictor(dlib_gray_image, face);
        switch (shape.num_parts()) {
        case 68:
            // assume iBUG 300-W dataset
            for (const auto &elem : Facemark_LUT) {
                std::vector<Point2i> points;

                for (size_t i = elem.second.first; i <= elem.second.second; i++) {
                    points.emplace_back(static_cast<int>(shape.part(i).x()),
                                        static_cast<int>(shape.part(i).y()));
                }
                marks.landmarks.emplace_back(std::make_pair(elem.first, points));
            }
            break;
        case 5:
            // asssume reduced dlib dataset
            left_eye_points.emplace_back(static_cast<int>(shape.part(0).x()),
                                         static_cast<int>(shape.part(0).y()));
            left_eye_points.emplace_back(static_cast<int>(shape.part(1).x()),
                                         static_cast<int>(shape.part(1).y()));
            right_eye_points.emplace_back(static_cast<int>(shape.part(2).x()),
                                          static_cast<int>(shape.part(2).y()));
            right_eye_points.emplace_back(static_cast<int>(shape.part(3).x()),
                                          static_cast<int>(shape.part(3).y()));
            nose_points.emplace_back(static_cast<int>(shape.part(4).x()),
                                     static_cast<int>(shape.part(4).y()));
            marks.landmarks.emplace_back(std::make_pair(FacemarkType::NOSE, nose_points));
            marks.landmarks.emplace_back(std::make_pair(FacemarkType::RIGHT_EYE, right_eye_points));
            marks.landmarks.emplace_back(std::make_pair(FacemarkType::LEFT_EYE, left_eye_points));
            break;
        default:
            // cannot handle this shape
            return false;
        }

        facemarks.push_back(marks);
    }

    return true;
}

bool KazemiFacemarkDetector::set_target(Target target) {
    std::unique_lock<std::mutex> lock(m_target_mutex);

    switch (target) {
    case Target::CPU:
        m_target = target;
        break;
    default:
        /* unsupported */
        return false;
    }

    return true;
}
