/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IPC_MODULE_H
#define SPH_IPC_MODULE_H

#include <seraphim/core/module.h>

namespace sph {
namespace ipc {

enum CLASSES { CLASS_SharedMemoryTransport = 0, CLASS_TCPTransport };

} // namespace ipc
} // namespace sph

#endif // SPH_IPC_MODULE_H
