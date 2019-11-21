/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IOP_H
#define SPH_IOP_H

#ifdef WITH_IOP_OPENCV
#include <seraphim/iop/opencv/mat.h>
#endif

#ifdef WITH_IOP_QT
#include <seraphim/iop/qt/qimage.h>
#endif

#endif // SPH_IOP_H
