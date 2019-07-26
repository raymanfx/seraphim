/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "seraphim/ipc/net/tcp_socket.h"

using namespace sph::ipc::net;

TCPSocket::TCPSocket(const int &domain) : Socket(domain, SOCK_STREAM, IPPROTO_TCP) {
    // allow address reuse by default
    int opt_val = 1;
    set_opt(SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
}

TCPSocket::~TCPSocket() {
    // dummy
}

bool TCPSocket::listen(const int &backlog) {
    return ::listen(m_fd, backlog) == 0;
}

int TCPSocket::accept(struct sockaddr *addr, socklen_t *addrlen) {
    return ::accept(m_fd, addr, addrlen);
}

ssize_t TCPSocket::receive(const int &fd, void *buf, const size_t &data_len_max, const int &flags) {
    ssize_t ret;

    ret = recv(fd, buf, data_len_max, flags);
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            m_state = STATE_TIMEOUT;
        } else {
            m_state = STATE_ERROR;
        }
    }

    m_state = STATE_OK;
    return ret;
}

ssize_t TCPSocket::receive_msg(const int &fd, struct msghdr *msg, const int &flags) {
    ssize_t ret;

    ret = recvmsg(fd, msg, flags);
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            m_state = STATE_TIMEOUT;
        } else {
            m_state = STATE_ERROR;
        }
    }

    m_state = STATE_OK;
    return ret;
}

ssize_t TCPSocket::send(const int &fd, const void *buf, const size_t &data_len, const int &flags) {
    ssize_t ret;

    ret = ::send(fd, buf, data_len, flags);
    if (ret == -1) {
        m_state = STATE_ERROR;
    }

    m_state = STATE_OK;
    return ret;
}

ssize_t TCPSocket::send_msg(const int &fd, struct msghdr *msg, const int &flags) {
    ssize_t ret;

    ret = sendmsg(fd, msg, flags);
    if (ret == -1) {
        m_state = STATE_ERROR;
    }

    m_state = STATE_OK;
    return ret;
}
