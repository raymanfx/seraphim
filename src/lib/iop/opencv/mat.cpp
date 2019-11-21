/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <opencv2/imgproc.hpp>
#include <seraphim/iop/opencv/mat.h>

::cv::Mat sph::iop::cv::from_image(const sph::Image &img) {
    ::cv::Mat mat;
    int type = 0;

    if (img.empty()) {
        return ::cv::Mat();
    }

    switch (img.pixfmt().channels()) {
    case 1:
        switch (img.pixfmt().size) {
        case 1:
            type = CV_8UC1;
            break;
        case 2:
            type = CV_16UC1;
            break;
        }
        break;
    case 3:
        switch (img.pixfmt().size) {
        case 1:
            type = CV_8UC3;
            break;
        case 2:
            type = CV_16UC3;
            break;
        }
        break;
    }

    if (type == 0) {
        return ::cv::Mat();
    }

    auto data = const_cast<unsigned char *>(reinterpret_cast<const unsigned char *>(img.data()));

    // https://github.com/opencv/opencv/blob/master/modules/videoio/src/cap_v4l.cpp
    switch (img.pixfmt().color) {
    case sph::Pixelformat::Color::GRAY:
    case sph::Pixelformat::Color::BGR:
    case sph::Pixelformat::Color::RGB:
        mat = ::cv::Mat(static_cast<int>(img.height()), static_cast<int>(img.width()), type, data,
                        img.stride());
        break;
    }

    return mat;
}

sph::CoreImage sph::iop::cv::to_image(const ::cv::Mat &mat) {
    uint32_t width;
    uint32_t height;
    size_t stride;
    sph::Pixelformat pixfmt;

    if (mat.empty()) {
        return sph::CoreImage();
    }

    // OpenCV images are always BGR
    pixfmt.color = Pixelformat::Color::BGR;
    pixfmt.size = mat.elemSize();

    width = static_cast<uint32_t>(mat.step / mat.elemSize());
    height = static_cast<uint32_t>(mat.rows);
    stride = mat.step;

    return sph::CoreImage(reinterpret_cast<std::byte *>(mat.data), width, height, pixfmt, stride);
}
