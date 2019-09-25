/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IPC_NET_TCP_SOCKET_H
#define SPH_IPC_NET_TCP_SOCKET_H

#include <arpa/inet.h>
#include <cstdint>
#include <string>
#include <unistd.h>
#include <vector>

#include "socket.h"

namespace sph {
namespace ipc {
namespace net {

/**
 * @brief Transmission Control Protocol socket.
 *
 * Allows for reliable peer-to-peer inter-process (and inter-machine) communication.
 */
class TCPSocket : public Socket {
public:
    explicit TCPSocket(const Family &family);

    /**
     * @brief Listen for incoming connection requests.
     *        Throws sph::core::RuntimeException when the OS socket op fails.
     * @param backlog Number of pending connections allowed.
     */
    void listen(const int &backlog);

    /**
     * @brief Accept incoming connection requests.
     *        Throws sph::core::RuntimeException when the OS socket op fails.
     * @param addr OS socket address.
     * @param addrlen OS socket address length (differs between IPv4 and IPv6).
     * @return File descriptor of the connected peer.
     */
    int accept(struct sockaddr *addr, socklen_t *addrlen);

    using Socket::receive;
    using Socket::receive_msg;
    using Socket::send;
    using Socket::send_msg;

    /**
     * @brief Receive an arbitrary number of bytes.
     *        Throws sph::core::RuntimeException when the OS socket connection fails.
     *        Throws sph::core::TimeoutException when the OS socket connection times out.
     *        Throws sph::ipc::PeerDisconnectedException when a peer disconnects.
     * @param fd Peer file descriptor.
     * @param buf Output buffer.
     * @param max_len Maximum number of bytes to receive.
     * @param flags OS socket flags.
     * @return Number of bytes received.
     */
    ssize_t receive(const int &fd, void *data, const size_t &data_len_max, const int &flags = 0);

    /**
     * @brief Transmit a number of bytes.
     *        Throws sph::core::RuntimeException when the OS socket connection fails.
     *        Throws sph::core::TimeoutException when the OS socket connection times out.
     * @param fd Peer file descriptor.
     * @param buf Input buffer.
     * @param len Number of bytes to send. Must not be larger than the input buffer length.
     * @param flags OS socket flags.
     * @return Number of bytes sent.
     */
    ssize_t send(const int &fd, const void *data, const size_t &data_len, const int &flags = 0);

    /**
     * @brief Gathering (vectored receival) of data.
     *        Throws sph::core::RuntimeException when the OS socket connection fails.
     *        Throws sph::core::TimeoutException when the OS socket connection times out.
     *        Throws sph::ipc::PeerDisconnectedException when a peer disconnects.
     * @param fd Peer file descriptor.
     * @param msg Message structure.
     * @param flags OS socket flags.
     * @return Number of bytes received.
     */
    ssize_t receive_msg(const int &fd, struct msghdr *msg, const int &flags = 0);

    /**
     * @brief Scattering (vectored transmission) of data.
     *        Throws sph::core::RuntimeException when the OS socket connection fails.
     *        Throws sph::core::TimeoutException when the OS socket connection times out.
     * @param fd Peer file descriptor.
     * @param msg Message structure.
     * @param flags OS socket flags.
     * @return Number of bytes sent.
     */
    ssize_t send_msg(const int &fd, struct msghdr *msg, const int &flags = 0);
};

} // namespace net
} // namespace ipc
} // namespace sph

#endif // SPH_IPC_NET_TCP_SOCKET_H
