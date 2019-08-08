/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdarg>
#include <seraphim/face.h>
#include <seraphim/face/module.h>

using namespace sph::face;

extern "C" void *create(int clazz, ...) {
    va_list args;
    va_start(args, clazz);

    switch (clazz) {
    case CLASS_LBPFaceDetector:
        return new LBPFaceDetector;
    case CLASS_LBPFaceRecognizer:
        return new LBPFaceRecognizer;
    case CLASS_LBFFacemarkDetector:
        return new LBFFacemarkDetector(
            std::shared_ptr<IFaceDetector>(static_cast<IFaceDetector *>(va_arg(args, void *))));
    case CLASS_HOGFaceDetector:
        return new HOGFaceDetector;
    }

    return nullptr;
}

extern "C" void destroy(int clazz, void *ptr) {
    switch (clazz) {
    case CLASS_LBPFaceDetector:
        delete reinterpret_cast<LBPFaceDetector *>(ptr);
        break;
    case CLASS_LBPFaceRecognizer:
        delete reinterpret_cast<LBPFaceRecognizer *>(ptr);
        break;
    case CLASS_LBFFacemarkDetector:
        delete reinterpret_cast<LBFFacemarkDetector *>(ptr);
        break;
    case CLASS_HOGFaceDetector:
        delete reinterpret_cast<HOGFaceDetector *>(ptr);
        break;
    }
}

static seraphim_module_factory FACTORY = { create, destroy };

static seraphim_module_version VERSION = {
    0, // major
    0, // minor
    0  // patch
};

static seraphim_module MODULE = {
    "face",  // id
    VERSION, // version
    FACTORY  // factory
};

extern "C" seraphim_module seraphim_module() {
    return MODULE;
}
