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
    QImage::Format fmt = QImage::Format_Invalid;

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
    switch (src.pixfmt().color) {
    case Pixelformat::Color::GRAY:
        switch (src.pixfmt().size) {
        case 1:
            fmt = QImage::Format_Grayscale8;
            break;
            // case 2:
            //    fmt = QImage::Format_Grayscale16;
            //    break;
        }
        break;
    case Pixelformat::Color::BGR:
    case Pixelformat::Color::RGB:
        switch (src.pixfmt().size) {
        case 1:
            fmt = QImage::Format_RGB888;
            break;
        case 3:
            fmt = QImage::Format_RGB888;
            break;
        }
        break;
    }

    if (fmt == QImage::Format_Invalid) {
        return false;
    }

    dst = QImage(bytes, static_cast<int>(src.width()), static_cast<int>(src.height()), fmt);
    if (src.pixfmt().color == Pixelformat::Color::BGR) {
        dst = dst.rgbSwapped();
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
