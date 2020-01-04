/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IPC_TRANSPORT_HPP
#define SPH_IPC_TRANSPORT_HPP

#include <Seraphim.pb.h>

#include "seraphim/threading.hpp"

namespace sph {
namespace ipc {

/**
 * @brief Message transport (IPC) interface.
 *
 * Derive from this class to implement a message transport.
 * The interface provides basic send and receive methods that must be implemented and operate on
 * Seraphim messages (which are protobuf messages).
 */
class Transport : public Synchronizeable<Transport> {
public:
    virtual ~Transport() = default;

    /**
     * @brief Set timeout for blocking RX operations.
     *        Throws sph::RuntimeException in case of errors.
     * @param ms Timeout in milliseconds, 0 means blocking.
     */
    virtual void set_rx_timeout(int ms) = 0;

    /**
     * @brief Set timeout for blocking TX operations.
     *        Throws sph::RuntimeException in case of errors.
     * @param ms Timeout in milliseconds, 0 means blocking.
     */
    virtual void set_tx_timeout(int ms) = 0;

    /**
     * @brief Receive a message.
     *        Throws sph::RuntimeException in case of errors.
     *        Throws sph::TimeoutException in case of timeouts.
     * @param msg The message, used as output parameter.
     */
    virtual void receive(Seraphim::Message &msg) = 0;

    /**
     * @brief Send a message.
     *        Throws sph::RuntimeException in case of errors.
     *        Throws sph::TimeoutException in case of timeouts.
     * @param msg The message.
     */
    virtual void send(const Seraphim::Message &msg) = 0;
};

} // namespace ipc
} // namespace sph

#endif // SPH_IPC_TRANSPORT_HPP
