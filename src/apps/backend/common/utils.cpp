/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstring>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <seraphim/core/image.h>
#include <seraphim/iop/opencv/mat.h>

#include "utils.h"

bool sph::backend::Image2DtoImage(const Seraphim::Types::Image2D &src, sph::core::Image &dst) {
    sph::core::Image img;
    sph::core::ImageBuffer::Format fmt = {};

    fmt.width = src.width();
    fmt.height = src.height();
    fmt.stride = src.stride();
    fmt.pixfmt = sph::core::ImageBuffer::as_pixelformat(src.fourcc());

    if (!img.mutable_buffer().assign(
            const_cast<unsigned char *>(
                reinterpret_cast<const unsigned char *>(src.data().c_str())),
            fmt)) {
        return false;
    }

    dst = img;
    return true;
}

bool sph::backend::Image2DtoMat(const Seraphim::Types::Image2D &src, cv::Mat &dst) {
    // create intermediate wrapper
    sph::core::Image img;

    if (!Image2DtoImage(src, img)) {
        return false;
    }

    dst = sph::iop::cv::MatFacility::from_image(img);
    return !dst.empty();
}
