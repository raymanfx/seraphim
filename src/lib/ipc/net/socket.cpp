/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <algorithm>
#include <cassert>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "seraphim/except.hpp"
#include "seraphim/ipc/net/socket.hpp"

using namespace sph;
using namespace sph::ipc::net;

Socket::Socket(Family family, Type type, Protocol protocol) {
    switch (family) {
    case Family::INET:
        m_family = AF_INET;
        break;
    case Family::INET6:
        m_family = AF_INET6;
        break;
    }

    if (m_family == -1) {
        SPH_THROW(InvalidArgumentException, "Family not implemented.");
    }

    switch (type) {
    case Type::STREAM:
        m_type = SOCK_STREAM;
        break;
    case Type::DATAGRAM:
        m_type = SOCK_DGRAM;
        break;
    }

    if (m_type == -1) {
        SPH_THROW(InvalidArgumentException, "Type not implemented.");
    }

    switch (protocol) {
    case Protocol::TCP:
        m_protocol = IPPROTO_TCP;
        break;
    case Protocol::UDP:
        m_protocol = IPPROTO_UDP;
        break;
    }

    if (m_protocol == -1) {
        SPH_THROW(InvalidArgumentException, "Protocol not implemented.");
    }

    // initialize the socket
    reset();
}

Socket::~Socket() {
    try {
        shutdown(Shutdown::BOTH);
    } catch (...) {
        // the socket will be dead after this point
    }

    ::close(m_fd);
}

void Socket::reset(bool keep_opts) {
    int new_fd;
    void *opt_val = nullptr;
    socklen_t opt_len;

    // cancel pending read/write ops
    if (m_fd > -1) {
        shutdown(Shutdown::BOTH);
    }

    // reset state
    m_bound = false;
    m_connected = false;

    // create the actual OS socket
    new_fd = socket(m_family, m_type, m_protocol);
    if (new_fd == -1) {
        SPH_THROW(RuntimeException, err_str());
    }

    if (keep_opts) {
        // apply current opts
        for (int &opt_name : m_socket_opts) {
            get_opt(SOL_SOCKET, opt_name, opt_val, &opt_len);
            if (setsockopt(new_fd, SOL_SOCKET, opt_name, opt_val, opt_len) == -1) {
                SPH_THROW(RuntimeException, err_str());
            }
        }
    } else {
        // reset opts
        m_socket_opts.clear();
    }

    ::close(m_fd);
    m_fd = new_fd;
}

bool Socket::bind(uint16_t port) {
    struct addrinfo *res;
    struct addrinfo hints = {};
    int gai_rc;

    hints.ai_family = m_family;
    hints.ai_socktype = m_type;
    hints.ai_flags = AI_PASSIVE;

    // get target address in network order
    gai_rc = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &res);
    if (gai_rc != 0) {
        SPH_THROW(RuntimeException, gai_strerror(gai_rc));
    }

    // get a new socket if required
    if (m_bound || m_connected) {
        reset(true);
    }

    if (::bind(m_fd, res->ai_addr, res->ai_addrlen) == -1) {
        freeaddrinfo(res);
        // failing to bind is not an 'exceptional' failure
        return false;
    }

    freeaddrinfo(res);
    m_bound = true;
    return true;
}

bool Socket::connect(const std::string &ipaddr, uint16_t port, int timeout) {
    struct addrinfo *res;
    struct addrinfo hints = {};
    int rc;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = m_type;

    // get target address in network order
    rc = getaddrinfo(ipaddr.c_str(), std::to_string(port).c_str(), &hints, &res);
    if (rc != 0) {
        SPH_THROW(RuntimeException, gai_strerror(rc));
    }

    // get a new socket if required
    if (m_bound || m_connected) {
        reset(true);
    }

    if (timeout > 0) {
        // set the socket as non-blocking, use select to see whether the
        // connect() call is successful, and finally set the socket as blocking
        // again
        fd_set fdset;
        struct timeval tv;
        int socket_opts;

        socket_opts = fcntl(m_fd, F_GETFL);
        if (socket_opts == -1) {
            freeaddrinfo(res);
            SPH_THROW(RuntimeException, err_str());
        }

        socket_opts |= O_NONBLOCK;
        fcntl(m_fd, F_SETFL, socket_opts);

        // we will read out the error code later
        ::connect(m_fd, res->ai_addr, res->ai_addrlen);

        FD_ZERO(&fdset);
        FD_SET(m_fd, &fdset);
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        rc = select(m_fd + 1, nullptr, &fdset, nullptr, &tv);
        if (rc == -1) {
            SPH_THROW(RuntimeException, err_str());
        } else if (rc == 0) {
            // select timeout
            SPH_THROW(TimeoutException);
        }

        int so_error;
        socklen_t len = sizeof(so_error);

        getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &so_error, &len);
        if (so_error != 0) {
            // the connection was not successful
            freeaddrinfo(res);

            // failing to connect is not an 'exceptional' failure
            return false;
        }

        // success, now make socket operations blocking again
        socket_opts &= ~O_NONBLOCK;
        if (fcntl(m_fd, F_SETFL, socket_opts) == -1) {
            freeaddrinfo(res);
            SPH_THROW(RuntimeException, err_str());
        }
    } else {
        // timeout not set, use blocking operation on socket
        if (::connect(m_fd, res->ai_addr, res->ai_addrlen) != 0) {
            freeaddrinfo(res);
            return false;
        }
    }

    freeaddrinfo(res);
    m_connected = true;
    return true;
}

void Socket::shutdown(Shutdown how) {
    int how_;

    switch (how) {
    case Shutdown::READ:
        how_ = SHUT_RD;
        break;
    case Shutdown::WRITE:
        how_ = SHUT_WR;
        break;
    case Shutdown::BOTH:
        how_ = SHUT_RDWR;
        break;
    }

    if (::shutdown(m_fd, how_) == -1) {
        SPH_THROW(RuntimeException, err_str());
    }
}

bool Socket::poll(int timeout) const {
    struct pollfd poll_fds[1];
    int poll_rc;

    // set up polling on socket
    poll_fds[0].fd = m_fd;
    poll_fds[0].events = POLLIN;

    // poll for <timeout>, then exit
    poll_rc = ::poll(poll_fds, 1, timeout);
    if (poll_rc == -1) {
        SPH_THROW(RuntimeException, err_str());
    }

    return poll_rc > 0;
}

void Socket::set_rx_timeout(long us) {
    struct timeval timeout;
    time_t secs = us / 1000000;
    suseconds_t usecs = (us * 1000000 - secs) / 1000000;

    timeout.tv_sec = secs;
    timeout.tv_usec = usecs;

    set_opt(SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

void Socket::set_tx_timeout(long us) {
    struct timeval timeout;
    time_t secs = us / 1000000;
    suseconds_t usecs = (us * 1000000 - secs) / 1000000;

    timeout.tv_sec = secs;
    timeout.tv_usec = usecs;

    set_opt(SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
}

ssize_t Socket::receive(void *buf, size_t max_len, int flags) {
    ssize_t ret;

    ret = recv(m_fd, buf, max_len, flags);
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            SPH_THROW(TimeoutException);
        } else {
            SPH_THROW(RuntimeException, err_str());
        }
    }

    return ret;
}

ssize_t Socket::send(const void *buf, size_t len, int flags) {
    ssize_t ret;

    ret = ::send(m_fd, buf, len, flags);
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            SPH_THROW(TimeoutException);
        } else {
            SPH_THROW(RuntimeException, err_str());
        }
    }

    return ret;
}

ssize_t Socket::receive_msg(struct msghdr *msg, int flags) {
    ssize_t ret;

    ret = recvmsg(m_fd, msg, flags);
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            SPH_THROW(TimeoutException);
        } else {
            SPH_THROW(RuntimeException, err_str());
        }
    }

    return ret;
}

ssize_t Socket::send_msg(struct msghdr *msg, int flags) {
    ssize_t ret;

    ret = sendmsg(m_fd, msg, flags);
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            SPH_THROW(TimeoutException);
        } else {
            SPH_THROW(RuntimeException, err_str());
        }
    }

    return ret;
}

void Socket::get_opt(int level, int opt_name, void *opt_val, socklen_t *opt_len) const {
    if (getsockopt(m_fd, level, opt_name, opt_val, opt_len) == -1) {
        SPH_THROW(RuntimeException, err_str());
    }
}

void Socket::set_opt(int level, int opt_name, const void *opt_val, socklen_t opt_len) {
    if (setsockopt(m_fd, level, opt_name, opt_val, opt_len) == -1) {
        SPH_THROW(RuntimeException, err_str());
    }

    m_socket_opts.push_back(opt_name);
}
