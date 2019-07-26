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

class TCPSocket : public Socket {
public:
    explicit TCPSocket(const int &domain);
    ~TCPSocket();

    /* listen for clients */
    bool listen(const int &backlog);

    /* accept incoming connections */
    int accept(struct sockaddr *addr, socklen_t *addrlen);

    /* RX and TX */
    using Socket::receive;
    using Socket::receive_msg;
    using Socket::send;
    using Socket::send_msg;

    /* RX and TX with arbitrary FDs */
    ssize_t receive(const int &fd, void *data, const size_t &data_len_max, const int &flags = 0);
    ssize_t receive_msg(const int &fd, struct msghdr *msg, const int &flags = 0);
    ssize_t send(const int &fd, const void *data, const size_t &data_len, const int &flags = 0);
    ssize_t send_msg(const int &fd, struct msghdr *msg, const int &flags = 0);
};

} // namespace net
} // namespace ipc
} // namespace sph

#endif // SPH_IPC_NET_TCP_SOCKET_H
