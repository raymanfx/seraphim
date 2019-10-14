/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IPC_EXCEPT_H
#define SPH_IPC_EXCEPT_H

#include "seraphim/core/except.h"

namespace sph {
namespace ipc {

/**
 * @brief Peer disconnection exception.
 *
 * Throw this exception whenever a previously connected peer disconnects (e.g. recv returns 0).
 */
class PeerDisconnectedException : public sph::RuntimeException {
    using RuntimeException::RuntimeException;
};

} // namespace ipc
} // namespace sph

#endif // SPH_IPC_EXCEPT_H
