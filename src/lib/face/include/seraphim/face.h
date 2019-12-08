/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_H
#define SPH_FACE_H

#include <seraphim/face/face_detector.h>
#include <seraphim/face/face_recognizer.h>
#include <seraphim/face/facemark_detector.h>

#ifdef WITH_OPENCV
#include <seraphim/face/lbf_facemark_detector.h>
#include <seraphim/face/lbp_face_detector.h>
#include <seraphim/face/lbp_face_recognizer.h>
#include <seraphim/face/utils.h>
#endif

#ifdef WITH_DLIB
#include <seraphim/face/hog_face_detector.h>
#include <seraphim/face/kazemi_facemark_detector.h>
#endif

#endif // SPH_FACE_H
