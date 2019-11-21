/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <seraphim/iop/qt/qimage.h>

using namespace sph;

QImage iop::qt::from_image(const Image &img) {
    QImage qimg;
    uint32_t row_alignment;
    QImage::Format fmt = QImage::Format_Invalid;

    if (img.empty()) {
        return QImage();
    }

    auto bytes = reinterpret_cast<const uchar *>(img.data());

    // QImage data must be 32-bit aligned, as must each individual scanline
    // ref: https://doc.qt.io/qt-5/qimage.html#QImage-3
    row_alignment = img.stride() & 3 ? 1 : 4;
    if (row_alignment != 4) {
        return QImage();
    }

    // https://doc.qt.io/qt-5/qvideoframe.html#PixelFormat-enum
    switch (img.pixfmt().pattern) {
    case Pixelformat::Pattern::MONO:
        switch (img.pixfmt().size) {
        case 1:
            fmt = QImage::Format_Grayscale8;
            break;
            // case 2:
            //    fmt = QImage::Format_Grayscale16;
            //    break;
        }
        break;
    case Pixelformat::Pattern::BGR:
    case Pixelformat::Pattern::RGB:
        switch (img.pixfmt().size) {
        case 1:
            fmt = QImage::Format_RGB888;
            break;
        case 3:
            fmt = QImage::Format_RGB888;
            break;
        }
        break;
    default:
        return QImage();
    }

    qimg = QImage(bytes, static_cast<int>(img.width()), static_cast<int>(img.height()), fmt);
    if (img.pixfmt().pattern == Pixelformat::Pattern::BGR) {
        qimg = qimg.rgbSwapped();
    }

    return qimg;
}

CoreImage iop::qt::to_image(const QImage &qimg) {
    Pixelformat pixfmt;

    if (qimg.isNull()) {
        return CoreImage();
    }

    switch (qimg.format()) {
    case QImage::Format_Grayscale8:
        pixfmt.pattern = Pixelformat::Pattern::MONO;
        pixfmt.size = 1;
        break;
    case QImage::Format_RGB888:
        pixfmt.pattern = Pixelformat::Pattern::RGB;
        pixfmt.size = 3;
        break;
    default:
        break;
    }

    return CoreImage(const_cast<uchar *>(qimg.bits()), static_cast<uint32_t>(qimg.width()),
                     static_cast<uint32_t>(qimg.height()), pixfmt,
                     static_cast<size_t>(qimg.bytesPerLine()));
}
