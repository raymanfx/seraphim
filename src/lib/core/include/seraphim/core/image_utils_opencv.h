/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_IMAGE_UTILS_OPENCV_H
#define SPH_CORE_IMAGE_UTILS_OPENCV_H

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <seraphim/core/image.h>

namespace sph {
namespace core {

static bool Image2Mat(const sph::core::Image &src, cv::Mat &dst) {
    if (src.empty()) {
        return false;
    }

    // https://github.com/opencv/opencv/blob/master/modules/videoio/src/cap_v4l.cpp
    switch (src.buffer().format().pixfmt) {
    case sph::core::ImageBuffer::Pixelformat::BGR24:
    case sph::core::ImageBuffer::Pixelformat::BGR32:
        dst = cv::Mat(static_cast<int>(src.height()), static_cast<int>(src.width()), CV_8UC3,
                      const_cast<unsigned char *>(src.buffer().data()));
        break;
    case sph::core::ImageBuffer::Pixelformat::RGB24:
    case sph::core::ImageBuffer::Pixelformat::RGB32:
        dst = cv::Mat(static_cast<int>(src.height()), static_cast<int>(src.width()), CV_8UC3,
                      const_cast<unsigned char *>(src.buffer().data()))
                  .clone();
        cv::cvtColor(dst, dst, cv::COLOR_RGB2BGR);
        break;
    case sph::core::ImageBuffer::Pixelformat::Y16:
        dst = cv::Mat(static_cast<int>(src.height()), static_cast<int>(src.width()), CV_16UC1,
                      const_cast<unsigned char *>(src.buffer().data()));
        break;
    default:
        return false;
    }

    return !dst.empty();
}

static bool Mat2Image(cv::InputArray src, sph::core::Image &dst) {
    cv::Mat mat;
    sph::core::ImageBuffer::Format fmt = {};

    if (src.isMat()) {
        mat = src.getMat();
    } else if (src.isUMat()) {
        mat = src.getUMat().getMat(cv::ACCESS_READ);
    } else {
        return false;
    }

    if (mat.empty()) {
        return false;
    }

    // OpenCV images are always BGR
    switch (mat.elemSize1()) {
    case 1:
        if (mat.channels() == 3) {
            fmt.pixfmt = sph::core::ImageBuffer::Pixelformat::BGR24;
        }
        break;
    case 2:
        if (mat.channels() == 1) {
            fmt.pixfmt = sph::core::ImageBuffer::Pixelformat::Y16;
        }
        break;
    default:
        return false;
    }

    fmt.width = static_cast<uint32_t>(mat.cols);
    fmt.height = static_cast<uint32_t>(mat.rows);
    fmt.padding = 0;
    return dst.mutable_buffer().assign(mat.data, fmt, false);
}

} // namespace core
} // namespace sph

#endif // SPH_CORE_IMAGE_UTILS_OPENCV_H
