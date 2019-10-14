/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <opencv2/imgproc.hpp>
#include <seraphim/iop/opencv/mat.h>

using namespace sph::iop::cv;

::cv::Mat MatFacility::from_image(const sph::Image &img) {
    ::cv::Mat mat;

    if (img.empty()) {
        return ::cv::Mat();
    }

    // https://github.com/opencv/opencv/blob/master/modules/videoio/src/cap_v4l.cpp
    switch (img.pixfmt()) {
    case sph::Pixelformat::Enum::BGR24:
        mat = ::cv::Mat(static_cast<int>(img.height()), static_cast<int>(img.width()), CV_8UC3,
                        const_cast<unsigned char *>(img.data()), img.stride());
        break;
    case sph::Pixelformat::Enum::RGB24:
        mat = ::cv::Mat(static_cast<int>(img.height()), static_cast<int>(img.width()), CV_8UC3,
                        const_cast<unsigned char *>(img.data()), img.stride())
                  .clone();
        ::cv::cvtColor(mat, mat, ::cv::COLOR_RGB2BGR);
        break;
    case sph::Pixelformat::Enum::GRAY8:
        mat = ::cv::Mat(static_cast<int>(img.height()), static_cast<int>(img.width()), CV_8UC1,
                        const_cast<unsigned char *>(img.data()), img.stride());
        break;
    case sph::Pixelformat::Enum::GRAY16:
        mat = ::cv::Mat(static_cast<int>(img.height()), static_cast<int>(img.width()), CV_16UC1,
                        const_cast<unsigned char *>(img.data()), img.stride());
        break;
    default:
        break;
    }

    return mat;
}

sph::Image MatFacility::to_image(const ::cv::Mat &mat) {
    uint32_t width;
    uint32_t height;
    size_t stride;
    sph::Pixelformat::Enum pixfmt;

    if (mat.empty()) {
        return sph::Image();
    }

    pixfmt = sph::Pixelformat::Enum::UNKNOWN;

    // OpenCV images are always BGR
    switch (mat.channels()) {
    case 3:
        if (mat.elemSize1() == 1) {
            pixfmt = sph::Pixelformat::Enum::BGR24;
        }
        break;
    case 1:
        if (mat.elemSize1() == 1) {
            pixfmt = sph::Pixelformat::Enum::GRAY8;
        } else if (mat.elemSize1() == 2) {
            pixfmt = sph::Pixelformat::Enum::GRAY16;
        }
        break;
    default:
        break;
    }

    if (pixfmt == sph::Pixelformat::Enum::UNKNOWN) {
        return sph::Image();
    }

    width = static_cast<uint32_t>(mat.step / mat.elemSize());
    height = static_cast<uint32_t>(mat.rows);
    stride = mat.step;

    return sph::Image(mat.data, width, height, pixfmt, stride);
}
