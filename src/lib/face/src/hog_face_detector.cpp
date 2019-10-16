/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <dlib/image_io.h>
#include <dlib/opencv/cv_image.h>
#include <seraphim/core/polygon.h>
#include <seraphim/iop/opencv/mat.h>

#include "seraphim/face/hog_face_detector.h"

using namespace sph;
using namespace sph::face;

HOGFaceDetector::HOGFaceDetector() {
    m_detector = dlib::get_frontal_face_detector();
}

bool HOGFaceDetector::detect(const Image &img, std::vector<Polygon<int>> &faces) {
    // http://dlib.net/face_detection_ex.cpp.html
    dlib::array2d<unsigned char> dlib_gray_image;
    std::vector<dlib::rectangle> dets;
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

    // Now tell the face detector to give us a list of bounding boxes
    // around all the faces it can find in the image.
    dets = m_detector(dlib_gray_image);

    for (auto const &box : dets) {
        faces.emplace_back(Polygon<int>(
            Point2i(static_cast<int>(box.bl_corner().x()), static_cast<int>(box.bl_corner().y())),
            Point2i(static_cast<int>(box.tl_corner().x()), static_cast<int>(box.tl_corner().y())),
            Point2i(static_cast<int>(box.tr_corner().x()), static_cast<int>(box.tr_corner().y())),
            Point2i(static_cast<int>(box.br_corner().x()), static_cast<int>(box.br_corner().y()))));
    }

    return true;
}

bool HOGFaceDetector::set_target(Target target) {
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
