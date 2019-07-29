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

static bool Image2Mat(const Image &src, cv::Mat &dst) {
    if (!src.valid()) {
        return false;
    }

    // https://github.com/opencv/opencv/blob/master/modules/videoio/src/cap_v4l.cpp
    switch (src.pixelformat()) {
    case Image::Pixelformat::FMT_BGR24:
        dst = cv::Mat(static_cast<int>(src.height()), static_cast<int>(src.width()), CV_8UC3,
                      const_cast<void *>(src.data()));
        break;
    case Image::Pixelformat::FMT_RGB24:
        cv::cvtColor(cv::Mat(static_cast<int>(src.height()), static_cast<int>(src.width()), CV_8UC3,
                             const_cast<void *>(src.data())),
                     dst, cv::COLOR_RGB2BGR);
        break;
    case Image::Pixelformat::FMT_YUYV:
        cv::cvtColor(cv::Mat(static_cast<int>(src.height()), static_cast<int>(src.width()), CV_8UC2,
                             const_cast<void *>(src.data())),
                     dst, cv::COLOR_YUV2BGR_YUYV);
        break;
    case Image::Pixelformat::FMT_MJPG:
        cv::imdecode(
            cv::Mat(1, static_cast<int>(src.data_size()), CV_8U, const_cast<void *>(src.data())),
            cv::IMREAD_COLOR, &dst);
        break;
    default:
        return false;
    }

    return true;
}

static bool Mat2Image(cv::InputArray src, Image &dst) {
    cv::Mat mat;
    sph::core::Image::Pixelformat fmt = sph::core::Image::Pixelformat::FMT_CUSTOM;

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
    switch (mat.elemSize()) {
    case 3:
        if (mat.channels() == 3) {
            fmt = sph::core::Image::Pixelformat::FMT_BGR24;
        }
        break;
    default:
        break;
    }

    dst = Image(static_cast<uint32_t>(mat.cols), static_cast<uint32_t>(mat.rows), 3 /* channels */);
    dst.wrap_data(mat.data, mat.total() * mat.elemSize(), fmt);

    return true;
}

} // namespace core
} // namespace sph

#endif // SPH_CORE_IMAGE_UTILS_OPENCV_H
