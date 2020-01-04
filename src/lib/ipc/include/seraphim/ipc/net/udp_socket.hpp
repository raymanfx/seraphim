/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IPC_NET_UDP_SOCKET_HPP
#define SPH_IPC_NET_UDP_SOCKET_HPP

#include <arpa/inet.h>
#include <cstdint>
#include <string>
#include <unistd.h>
#include <vector>

#include "socket.hpp"

namespace sph {
namespace ipc {
namespace net {

/**
 * @brief User Datagram Protocol socket.
 *
 * Allows sending of data over the network without required established connection (asynchronous).
 */
class UDPSocket : public Socket {
public:
    explicit UDPSocket(Family family);
};

} // namespace net
} // namespace ipc
} // namespace sph

#endif // SPH_IPC_NET_UDP_SOCKET_HPP
