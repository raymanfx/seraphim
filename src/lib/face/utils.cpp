/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include "seraphim/face/utils.hpp"

double sph::face::align_face(cv::InputOutputArray image, std::vector<cv::Point2f> &eyes) {
    cv::Point2f center;
    float dx;
    float dy;
    double angle;
    cv::Mat transform;

    if (eyes.size() != 2) {
        return 0.0;
    }

    // center between the eyes
    center = cv::Point2f((eyes[0].x + eyes[1].x) * 0.5f, (eyes[0].y + eyes[1].y) * 0.5f);

    // angle between the eyes
    dx = eyes[1].x - eyes[0].x;
    dy = eyes[1].y - eyes[0].y;
    angle = static_cast<double>(atan2f(dy, dx)) * 180.0 / CV_PI;

    // get the transformation matrix for rotation
    transform = cv::Mat(cv::getRotationMatrix2D(center, angle, 1.0));

    // apply transformation
    cv::warpAffine(image, image, transform, image.size());

    return angle;
}
