/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstring>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "utils.h"

bool sph::backend::Image2DtoMat(const Seraphim::Types::Image2D &img, cv::Mat &dst) {
    // https://github.com/opencv/opencv/blob/master/modules/videoio/src/cap_v4l.cpp
    switch (img.fourcc()) {
    case fourcc('Y', 'U', 'Y', 'V'):
        cv::cvtColor(
            cv::Mat(img.height(), img.width(), CV_8UC2, const_cast<char *>(img.data().c_str())),
            dst, cv::COLOR_YUV2BGR_YUYV);
        break;
    case fourcc('M', 'J', 'P', 'G'):
        cv::imdecode(cv::Mat(1, img.data().size(), CV_8U, const_cast<char *>(img.data().c_str())),
                     cv::IMREAD_COLOR, &dst);
        break;
    default:
        return false;
    }

    return !dst.empty();
}
