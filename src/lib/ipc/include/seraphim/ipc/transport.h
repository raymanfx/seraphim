/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IPC_TRANSPORT_H
#define SPH_IPC_TRANSPORT_H

#include <Seraphim.pb.h>

namespace sph {
namespace ipc {

/**
 * @brief Message transport (IPC) interface.
 *
 * Derive from this class to implement a message transport.
 * The interface provides basic send and receive methods that must be implemented and operate on
 * Seraphim messages (which are protobuf messages).
 */
class ITransport {
public:
    virtual ~ITransport() = default;

    /**
     * @brief Receive a message.
     * @param msg The message, used as output parameter.
     * @return true on success, false on error or timeout.
     */
    virtual bool recv(Seraphim::Message &msg) = 0;

    /**
     * @brief Send a message.
     * @param msg The message.
     * @return true on success, false on error or timeout.
     */
    virtual bool send(const Seraphim::Message &msg) = 0;

protected:
    ITransport() = default;
};

} // namespace ipc
} // namespace sph

#endif // SPH_IPC_TRANSPORT_H
