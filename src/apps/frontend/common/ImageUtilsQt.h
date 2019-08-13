/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FRONTEND_IMAGE_UTILS_QT_H
#define SPH_FRONTEND_IMAGE_UTILS_QT_H

#include <QImage>
#include <seraphim/core/image.h>

namespace sph {
namespace frontend {

static bool Image2QImage(const sph::core::Image &src, QImage &dst) {
    const uchar *bytes = static_cast<const uchar *>(src.buffer().data());

    if (src.empty()) {
        return false;
    }

    // QImage data must be 32-bit aligned, as must each individual scanline
    // ref: https://doc.qt.io/qt-5/qimage.html#QImage-3
    if (src.buffer().row_alignment() != 4) {
        return false;
    }

    // https://doc.qt.io/qt-5/qvideoframe.html#PixelFormat-enum
    switch (src.buffer().format().pixfmt) {
    case sph::core::ImageBuffer::Pixelformat::BGR24:
    case sph::core::ImageBuffer::Pixelformat::BGR32:
        dst = QImage(bytes, static_cast<int>(src.width()), static_cast<int>(src.height()),
                     QImage::Format_RGB888)
                  .rgbSwapped();
        break;
    case sph::core::ImageBuffer::Pixelformat::RGB24:
    case sph::core::ImageBuffer::Pixelformat::RGB32:
        dst = QImage(bytes, static_cast<int>(src.width()), static_cast<int>(src.height()),
                     QImage::Format_RGB888);
        break;
    default:
        return false;
    }

    return true;
}

static bool QImage2Image(QImage &src, sph::core::Image &dst) {
    // TODO: implement
    (void)src;
    (void)dst;
    return false;
}

} // namespace frontend
} // namespace sph

#endif // SPH_FRONTEND_IMAGE_UTILS_QT_H
