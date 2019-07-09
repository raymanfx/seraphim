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
    case CLASS_LBPDetector:
        return new LBPDetector;
    case CLASS_LBPRecognizer:
        return new LBPRecognizer;
    }

    return nullptr;
}

extern "C" void destroy(int clazz, void *ptr) {
    switch (clazz) {
    case CLASS_LBPDetector:
        delete reinterpret_cast<LBPDetector *>(ptr);
        break;
    case CLASS_LBPRecognizer:
        delete reinterpret_cast<LBPRecognizer *>(ptr);
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
