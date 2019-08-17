/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <opencv2/imgproc.hpp>
#include <seraphim/iop/opencv/mat.h>

using namespace sph::iop::cv;

::cv::Mat MatFacility::from_image(const sph::core::Image &img) {
    ::cv::Mat mat;

    if (img.empty()) {
        return mat;
    }

    // https://github.com/opencv/opencv/blob/master/modules/videoio/src/cap_v4l.cpp
    switch (img.buffer().format().pixfmt) {
    case sph::core::ImageBuffer::Pixelformat::BGR24:
        mat = ::cv::Mat(static_cast<int>(img.height()), static_cast<int>(img.width()), CV_8UC3,
                        const_cast<unsigned char *>(img.buffer().data()),
                        img.buffer().format().stride);
        break;
    case sph::core::ImageBuffer::Pixelformat::RGB24:
        mat = ::cv::Mat(static_cast<int>(img.height()), static_cast<int>(img.width()), CV_8UC3,
                        const_cast<unsigned char *>(img.buffer().data()),
                        img.buffer().format().stride)
                  .clone();
        ::cv::cvtColor(mat, mat, ::cv::COLOR_RGB2BGR);
        break;
    case sph::core::ImageBuffer::Pixelformat::Y8:
        mat = ::cv::Mat(static_cast<int>(img.height()), static_cast<int>(img.width()), CV_8UC1,
                        const_cast<unsigned char *>(img.buffer().data()),
                        img.buffer().format().stride);
        break;
    case sph::core::ImageBuffer::Pixelformat::Y16:
        mat = ::cv::Mat(static_cast<int>(img.height()), static_cast<int>(img.width()), CV_16UC1,
                        const_cast<unsigned char *>(img.buffer().data()),
                        img.buffer().format().stride);
        break;
    default:
        break;
    }

    return mat;
}

sph::core::Image MatFacility::to_image(const ::cv::Mat &mat) {
    sph::core::Image img;
    sph::core::ImageBuffer::Format fmt = {};

    if (mat.empty()) {
        return img;
    }

    fmt.pixfmt = sph::core::ImageBuffer::Pixelformat::UNKNOWN;

    // OpenCV images are always BGR
    switch (mat.channels()) {
    case 3:
        if (mat.elemSize1() == 1) {
            fmt.pixfmt = sph::core::ImageBuffer::Pixelformat::BGR24;
        }
        break;
    case 1:
        if (mat.elemSize1() == 1) {
            fmt.pixfmt = sph::core::ImageBuffer::Pixelformat::Y8;
        } else if (mat.elemSize1() == 2) {
            fmt.pixfmt = sph::core::ImageBuffer::Pixelformat::Y16;
        }
        break;
    default:
        break;
    }

    if (fmt.pixfmt == sph::core::ImageBuffer::Pixelformat::UNKNOWN) {
        return img;
    }

    fmt.width = static_cast<uint32_t>(mat.step / mat.elemSize());
    fmt.height = static_cast<uint32_t>(mat.rows);
    fmt.stride = mat.step;

    img.mutable_buffer().assign(mat.data, fmt);

    return img;
}
