/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_IMAGE_UTILS_QT_H
#define SPH_CORE_IMAGE_UTILS_QT_H

#include <QImage>
#include <seraphim/core/image.h>

namespace sph {
namespace core {

static bool Image2QImage(const Image &src, QImage &dst) {
    const uchar *bytes = static_cast<const uchar *>(src.data());

    if (!src.valid()) {
        return false;
    }

    // https://doc.qt.io/qt-5/qvideoframe.html#PixelFormat-enum
    switch (src.pixelformat()) {
    case Image::Pixelformat::FMT_RGB24:
        dst = QImage(bytes, static_cast<int>(src.width()), static_cast<int>(src.height()),
                     QImage::Format_RGB888);
        break;
    case Image::Pixelformat::FMT_MJPG:
        dst = QImage::fromData(bytes, static_cast<int>(src.data_size()), "mjpeg");
        break;
    case Image::Pixelformat::FMT_YUYV:
    case Image::Pixelformat::FMT_CUSTOM:
        return false;
    }

    return true;
}

static bool QImage2Image(QImage &src, Image &dst) {
    // TODO: implement
    (void)src;
    (void)dst;
    return false;
}

} // namespace core
} // namespace sph

#endif // SPH_CORE_IMAGE_UTILS_QT_H
