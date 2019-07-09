/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdarg>
#include <seraphim/object.h>
#include <seraphim/object/module.h>

using namespace sph::object;

extern "C" void *create(int clazz, ...) {
    va_list args;
    va_start(args, clazz);

    switch (clazz) {
    case CLASS_DNNClassifier:
        return new DNNClassifier;
    }

    return nullptr;
}

extern "C" void destroy(int clazz, void *ptr) {
    switch (clazz) {
    case CLASS_DNNClassifier:
        delete reinterpret_cast<DNNClassifier *>(ptr);
    }
}

static seraphim_module_factory FACTORY = { create, destroy };

static seraphim_module_version VERSION = {
    0, // major
    0, // minor
    0  // patch
};

static seraphim_module MODULE = {
    "object", // id
    VERSION,  // version
    FACTORY   // factory
};

extern "C" seraphim_module seraphim_module() {
    return MODULE;
}
