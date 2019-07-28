/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <dlib/image_io.h>
#include <dlib/opencv/cv_image.h>

#include "seraphim/face/hog_face_detector.h"

using namespace sph::face;

HOGFaceDetector::HOGFaceDetector() {
    m_detector = dlib::get_frontal_face_detector();

    m_target = TARGET_CPU;
}

HOGFaceDetector::~HOGFaceDetector() {
    // dummy
}

bool HOGFaceDetector::detect_faces(cv::InputArray img, cv::OutputArray ROIs) {
    // http://dlib.net/face_detection_ex.cpp.html
    dlib::array2d<dlib::bgr_pixel> dlib_bgr_image;
    dlib::array2d<unsigned char> dlib_gray_image;
    std::vector<dlib::rectangle> dets;
    std::vector<cv::Rect> faces;

    if (!img.isMat()) {
        return false;
    }

    // convert from cv to dlib image
    switch (img.channels()) {
    case 1:
        dlib::assign_image(dlib_gray_image,
                           dlib::cv_image<unsigned char>(img.getMat()));
        break;
    case 3:
        dlib::assign_image(dlib_bgr_image,
                           dlib::cv_image<dlib::bgr_pixel>(img.getMat()));
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
    switch (img.channels()) {
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
    switch (img.channels()) {
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
        faces.push_back(cv::Rect(cv::Point(box.bl_corner().x(), box.bl_corner().y()),
                                 cv::Point(box.tr_corner().x(), box.tr_corner().y())));
    }

    cv::Mat(faces).copyTo(ROIs);
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
