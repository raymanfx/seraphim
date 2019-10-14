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

using namespace sph;
using namespace sph::ipc::net;

TCPSocket::TCPSocket(Family family) : Socket(family, Socket::Type::STREAM, Socket::Protocol::TCP) {
    // allow address reuse by default
    int opt_val = 1;
    set_opt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt_val, sizeof(opt_val));
}

void TCPSocket::listen(int backlog) {
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

ssize_t TCPSocket::receive(int fd, void *buf, size_t data_len_max, int flags) {
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

ssize_t TCPSocket::send(int fd, const void *buf, size_t data_len, int flags) {
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

ssize_t TCPSocket::receive_msg(int fd, struct msghdr *msg, int flags) {
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

ssize_t TCPSocket::send_msg(int fd, struct msghdr *msg, int flags) {
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
