/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IOP_QT_QIMAGE_H
#define SPH_IOP_QT_QIMAGE_H

#include <QImage>
#include <seraphim/image.h>

namespace sph {
namespace iop {
namespace qt {

/**
 * @brief Convert a Seraphim Image to a QImage.
 * @param img The source image.
 * @return The converted QImage, which is empty if the conversion failed.
 */
QImage from_image(const Image &img);

/**
 * @brief Convert a QImage to a Seraphim Image.
 * @param qimg The source QImage.
 * @return The converted Image, which is empty if the conversion failed.
 */
CoreImage to_image(const QImage &qimg);

} // namespace qt
} // namespace iop
} // namespace sph

#endif // SPH_IOP_QT_QIMAGE_H
