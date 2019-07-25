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
     * @brief Input/Output operation result.
     * The boolean operator is overloaded so it can be treated like a binary value, in which case
     * true means "OK" and false everything else.
     */
    enum class IOResult { ERROR, OK, TIMEOUT };
    inline friend bool operator!(const IOResult &res) { return res != IOResult::OK; }

    /**
     * @brief Set timeout for blocking RX/TX operations.
     * @param ms Timeout in milliseconds, 0 means blocking.
     * @return true on success, false otherwise.
     */
    virtual void set_timeout(const int &ms) = 0;

    /**
     * @brief Receive a message.
     * @param msg The message, used as output parameter.
     * @return true on success, false on error or timeout.
     */
    virtual IOResult recv(Seraphim::Message &msg) = 0;

    /**
     * @brief Send a message.
     * @param msg The message.
     * @return true on success, false on error or timeout.
     */
    virtual IOResult send(const Seraphim::Message &msg) = 0;

protected:
    ITransport() = default;
};

} // namespace ipc
} // namespace sph

#endif // SPH_IPC_TRANSPORT_H
