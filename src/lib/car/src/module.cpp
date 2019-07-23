/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdarg>
#include <seraphim/car.h>
#include <seraphim/car/module.h>

using namespace sph::car;

extern "C" void *create(int clazz, ...) {
    va_list args;
    va_start(args, clazz);

    switch (clazz) {
    case CLASS_LinearLaneDetector:
        return new LinearLaneDetector;
    }

    return nullptr;
}

extern "C" void destroy(int clazz, void *ptr) {
    switch (clazz) {
    case CLASS_LinearLaneDetector:
        delete reinterpret_cast<LinearLaneDetector *>(ptr);
    }
}

static seraphim_module_factory FACTORY = { create, destroy };

static seraphim_module_version VERSION = {
    0, // major
    0, // minor
    0  // patch
};

static seraphim_module MODULE = {
    "car",   // id
    VERSION, // version
    FACTORY  // factory
};

extern "C" seraphim_module seraphim_module() {
    return MODULE;
}
