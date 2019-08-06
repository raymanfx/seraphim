/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <dlib/image_io.h>
#include <dlib/opencv/cv_image.h>
#include <seraphim/core/image_utils_opencv.h>
#include <seraphim/core/polygon.h>

#include "seraphim/face/hog_face_detector.h"

using namespace sph::core;
using namespace sph::face;

HOGFaceDetector::HOGFaceDetector() {
    m_detector = dlib::get_frontal_face_detector();

    m_target = TARGET_CPU;
}

HOGFaceDetector::~HOGFaceDetector() {
    // dummy
}

bool HOGFaceDetector::detect_faces(const Image &img, std::vector<Polygon<>> &faces) {
    // http://dlib.net/face_detection_ex.cpp.html
    dlib::array2d<dlib::bgr_pixel> dlib_bgr_image;
    dlib::array2d<unsigned char> dlib_gray_image;
    std::vector<dlib::rectangle> dets;
    cv::Mat mat;

    if (!sph::core::Image2Mat(img, mat)) {
        return false;
    }

    // convert from cv to dlib image
    switch (mat.channels()) {
    case 1:
        dlib::assign_image(dlib_gray_image, dlib::cv_image<unsigned char>(mat));
        break;
    case 3:
        dlib::assign_image(dlib_bgr_image, dlib::cv_image<dlib::bgr_pixel>(mat));
        break;
    default:
        return false;
    }

    // Make the image bigger by a factor of two.  This is useful since
    // the face detector looks for faces that are about 80 by 80 pixels
    // or larger.  Therefore, if you want to find faces that are smaller
    // than that then you need to upsample the image as we do here by
    // calling pyramid_up().  So this will allow it to detect faces that
    // are at least 40 by 40 pixels in size.  We could call pyramid_up()
    // again to find even smaller faces, but note that every time we
    // upsample the image we make the detector run slower since it must
    // process a larger image.
    switch (mat.channels()) {
    case 1:
        dlib::pyramid_up(dlib_gray_image);
        break;
    case 3:
        dlib::pyramid_up(dlib_bgr_image);
        break;
    default:
        return false;
    }

    // Now tell the face detector to give us a list of bounding boxes
    // around all the faces it can find in the image.
    switch (mat.channels()) {
    case 1:
        dets = m_detector(dlib_gray_image);
        break;
    case 3:
        dets = m_detector(dlib_bgr_image);
        break;
    default:
        return false;
    }

    for (auto const &box : dets) {
        faces.emplace_back(Polygon<>(
            { { static_cast<int>(box.bl_corner().x()), static_cast<int>(box.bl_corner().y()) },
              { static_cast<int>(box.tl_corner().x()), static_cast<int>(box.tl_corner().y()) },
              { static_cast<int>(box.tr_corner().x()), static_cast<int>(box.tr_corner().y()) },
              { static_cast<int>(box.br_corner().x()), static_cast<int>(box.br_corner().y()) } }));
    }

    return true;
}

bool HOGFaceDetector::set_target(const target_t &target) {
    std::unique_lock<std::mutex> lock(m_target_mutex);

    switch (target) {
    case TARGET_CPU:
        m_target = target;
        break;
    default:
        /* unsupported */
        return false;
    }

    return true;
}
