/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FRONTEND_IMAGE_UTILS_QT_H
#define SPH_FRONTEND_IMAGE_UTILS_QT_H

#include <QImage>
#include <seraphim/image.h>

namespace sph {
namespace frontend {

bool Image2QImage(const sph::Image &src, QImage &dst) {
    auto bytes = reinterpret_cast<const uchar *>(src.data());
    uint32_t row_alignment;

    if (src.empty()) {
        return false;
    }

    // QImage data must be 32-bit aligned, as must each individual scanline
    // ref: https://doc.qt.io/qt-5/qimage.html#QImage-3
    row_alignment = src.stride() & 3 ? 1 : 4;
    if (row_alignment != 4) {
        return false;
    }

    // https://doc.qt.io/qt-5/qvideoframe.html#PixelFormat-enum
    switch (src.pixfmt()) {
    case sph::Pixelformat::Enum::BGR24:
    case sph::Pixelformat::Enum::BGR32:
        dst = QImage(bytes, static_cast<int>(src.width()), static_cast<int>(src.height()),
                     QImage::Format_RGB888)
                  .rgbSwapped();
        break;
    case sph::Pixelformat::Enum::RGB24:
    case sph::Pixelformat::Enum::RGB32:
        dst = QImage(bytes, static_cast<int>(src.width()), static_cast<int>(src.height()),
                     QImage::Format_RGB888);
        break;
    default:
        return false;
    }

    return true;
}

bool QImage2Image(QImage &src, sph::Image &dst) {
    // TODO: implement
    (void)src;
    (void)dst;
    return false;
}

} // namespace frontend
} // namespace sph

#endif // SPH_FRONTEND_IMAGE_UTILS_QT_H
