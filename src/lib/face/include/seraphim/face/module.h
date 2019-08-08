/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_MODULE_H
#define SPH_FACE_MODULE_H

#include <seraphim/core/module.h>

namespace sph {
namespace face {

enum CLASSES { CLASS_LBPFaceDetector = 0, CLASS_LBPFaceRecognizer, CLASS_LBFFacemarkDetector, CLASS_HOGFaceDetector };

} // namespace face
} // namespace sph

#endif // SPH_FACE_MODULE_H
