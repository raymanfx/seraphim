/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdarg>
#include <seraphim/iop.h>

using namespace sph::iop;

extern "C" void *create(int clazz, ...) {
    va_list args;
    va_start(args, clazz);

    switch (clazz) {
#ifdef WITH_OPENCV
    case CLASS_CV_MatFacility:
        return &sph::iop::cv::MatFacility::Instance();
#endif
    }

    return nullptr;
}

extern "C" void destroy(int clazz, void *ptr) {
    switch (clazz) {
#ifdef WITH_OPENCV
    case CLASS_CV_MatFacility:
        // we returned a pointer to the singleton instance, so there's nothing to be done here
        break;
    }
#endif
}

static seraphim_module_factory FACTORY = { create, destroy };

static seraphim_module_version VERSION = {
    0, // major
    0, // minor
    0  // patch
};

static seraphim_module MODULE = {
    "iop",   // id
    VERSION, // version
    FACTORY  // factory
};

extern "C" seraphim_module seraphim_module() {
    return MODULE;
}
