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
#include <seraphim/core/image_utils_opencv.h>

#include "utils.h"

bool sph::backend::Image2DtoMat(const Seraphim::Types::Image2D &src, cv::Mat &dst) {
    // create intermediate wrapper
    sph::core::Image img(src.width(), src.height(), 3 /* channels */);
    sph::core::Image::Pixelformat fmt = sph::core::Image::as_pixelformat(src.fourcc());

    if (!img.wrap_data(const_cast<void *>(reinterpret_cast<const void *>(src.data().c_str())),
                       src.data().size(), fmt)) {
        return false;
    }

    if (!sph::core::Image2Mat(img, dst)) {
        return false;
    }

    return !dst.empty();
}
