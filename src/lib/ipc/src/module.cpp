/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdarg>
#include <seraphim/ipc.h>
#include <seraphim/ipc/module.h>

using namespace sph::ipc;

extern "C" void *create(int clazz, ...) {
    va_list args;
    va_start(args, clazz);

    switch (clazz) {
    case CLASS_SharedMemoryTransport:
        return new SharedMemoryTransport;
    case CLASS_TCPTransport:
        return new TCPTransport(va_arg(args, int));
    }

    return nullptr;
}

extern "C" void destroy(int clazz, void *ptr) {
    switch (clazz) {
    case CLASS_SharedMemoryTransport:
        delete reinterpret_cast<SharedMemoryTransport *>(ptr);
        break;
    case CLASS_TCPTransport:
        delete reinterpret_cast<TCPTransport *>(ptr);
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
    "ipc",   // id
    VERSION, // version
    FACTORY  // factory
};

extern "C" seraphim_module seraphim_module() {
    return MODULE;
}
