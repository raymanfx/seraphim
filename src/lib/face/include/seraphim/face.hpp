/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_HPP
#define SPH_FACE_HPP

#include <seraphim/face/face_detector.hpp>
#include <seraphim/face/face_recognizer.hpp>
#include <seraphim/face/facemark_detector.hpp>

#ifdef WITH_OPENCV
#include <seraphim/face/lbf_facemark_detector.hpp>
#include <seraphim/face/lbp_face_detector.hpp>
#include <seraphim/face/lbp_face_recognizer.hpp>
#include <seraphim/face/utils.hpp>
#endif

#ifdef WITH_DLIB
#include <seraphim/face/hog_face_detector.hpp>
#include <seraphim/face/kazemi_facemark_detector.hpp>
#endif

#endif // SPH_FACE_HPP
