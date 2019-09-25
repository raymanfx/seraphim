/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "seraphim/core/except.h"
#include "seraphim/ipc/except.h"
#include "seraphim/ipc/net/tcp_socket.h"

using namespace sph::core;
using namespace sph::ipc::net;

TCPSocket::TCPSocket(const Family &family)
    : Socket(family, Socket::Type::STREAM, Socket::Protocol::TCP) {
    // allow address reuse by default
    int opt_val = 1;
    set_opt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt_val, sizeof(opt_val));
}

void TCPSocket::listen(const int &backlog) {
    if (::listen(m_fd, backlog) == -1) {
        SPH_THROW(RuntimeException, strerror(errno));
    }
}

int TCPSocket::accept(struct sockaddr *addr, socklen_t *addrlen) {
    int fd;

    fd = ::accept(m_fd, addr, addrlen);
    if (fd == -1) {
        SPH_THROW(RuntimeException, strerror(errno));
    }

    return fd;
}

ssize_t TCPSocket::receive(const int &fd, void *buf, const size_t &data_len_max, const int &flags) {
    ssize_t ret;

    ret = recv(fd, buf, data_len_max, flags);
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            SPH_THROW(TimeoutException);
        } else {
            SPH_THROW(RuntimeException, strerror(errno));
        }
    } else if (ret == 0) {
        SPH_THROW(PeerDisconnectedException);
    }

    return ret;
}

ssize_t TCPSocket::send(const int &fd, const void *buf, const size_t &data_len, const int &flags) {
    ssize_t ret;

    ret = ::send(fd, buf, data_len, flags);
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            SPH_THROW(TimeoutException);
        } else {
            SPH_THROW(RuntimeException, strerror(errno));
        }
    }

    return ret;
}

ssize_t TCPSocket::receive_msg(const int &fd, struct msghdr *msg, const int &flags) {
    ssize_t ret;

    ret = recvmsg(fd, msg, flags);
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            SPH_THROW(TimeoutException);
        } else {
            SPH_THROW(RuntimeException, strerror(errno));
        }
    } else if (ret == 0) {
        SPH_THROW(PeerDisconnectedException);
    }

    return ret;
}

ssize_t TCPSocket::send_msg(const int &fd, struct msghdr *msg, const int &flags) {
    ssize_t ret;

    ret = sendmsg(fd, msg, flags);
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            SPH_THROW(TimeoutException);
        } else {
            SPH_THROW(RuntimeException, strerror(errno));
        }
    }

    return ret;
}
