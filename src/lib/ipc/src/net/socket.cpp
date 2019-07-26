/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <algorithm>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "seraphim/ipc/net/socket.h"

using namespace sph::ipc::net;

Socket::Socket(const int &domain, const int &type, const int &protocol) {
    /* socket attributes */
    m_domain = domain;
    m_type = type;
    m_protocol = protocol;

    /* socket state */
    m_state = STATE_OK;
    m_connected = false;
    m_bound = false;

    /* initial socket */
    m_fd = socket(m_domain, m_type, m_protocol);
}

Socket::~Socket() {
    if (m_fd > -1) {
        shutdown(SHUT_RDWR);
        close();
    }
}

void Socket::reset(const bool &keep_opts) {
    int new_fd;
    void *opt_val = nullptr;
    socklen_t opt_len;

    if (m_bound || m_connected) {
        shutdown(SHUT_RDWR);
    }

    // create new socket
    new_fd = socket(m_domain, m_type, m_protocol);

    // reset state
    m_bound = false;
    m_connected = false;

    if (m_fd > -1 && keep_opts) {
        // apply current opts
        for (int &opt_name : m_socket_opts) {
            if (getsockopt(m_fd, SOL_SOCKET, opt_name, opt_val, &opt_len) == -1 ||
                setsockopt(m_fd, SOL_SOCKET, opt_name, opt_val, opt_len) == -1) {
                // something went wrong, remove the option from the internal map
                std::vector<int>::iterator position =
                    std::find(m_socket_opts.begin(), m_socket_opts.end(), opt_name);
                if (position != m_socket_opts.end()) {
                    m_socket_opts.erase(position);
                }
            }
        }
    } else {
        // reset opts
        m_socket_opts.clear();
    }

    if (m_fd > -1) {
        ::close(m_fd);
    }

    m_fd = new_fd;
}

bool Socket::bind(const struct sockaddr &addr) {
    const socklen_t addr_len = sizeof(addr);
    struct sockaddr current_addr = {};
    socklen_t current_addr_len = sizeof(current_addr);
    uint16_t current_port = 0;

    // retrieve the address which is currently assigned to the socket
    if (getsockname(m_fd, &current_addr, &current_addr_len) == -1) {
        return false;
    }

    switch (current_addr.sa_family) {
    case AF_INET:
        current_port = reinterpret_cast<struct sockaddr_in *>(&current_addr)->sin_port;
        break;
    case AF_INET6:
        current_port = reinterpret_cast<struct sockaddr_in6 *>(&current_addr)->sin6_port;
        break;
    default:
        return false;
    }

    // check if already bound to the socket
    // TODO: can only check against 0 here, but maybe there is a better way?
    if (ntohs(current_port) != 0) {
        return true;
    }

    // get a new socket if required
    if (m_bound || m_connected) {
        reset(true);
    }

    // bind socket with address struct (one port on all interfaces)
    m_bound = ::bind(m_fd, &addr, addr_len) == 0;
    return m_bound;
}

bool Socket::bind(const uint16_t &port) {
    bool ret;
    struct addrinfo *address_info;
    struct addrinfo address_info_hints = {};

    address_info_hints.ai_family = m_domain;
    address_info_hints.ai_socktype = m_type;
    address_info_hints.ai_protocol = 0;
    address_info_hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;

    // get target address in network order
    if (getaddrinfo(nullptr, std::to_string(port).c_str(), &address_info_hints, &address_info) !=
        0) {
        return false;
    }

    ret = bind(*address_info->ai_addr);
    freeaddrinfo(address_info);
    return ret;
}

bool Socket::connect(const struct sockaddr &addr, const int &timeout) {
    const socklen_t addr_len = sizeof(addr);

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
            return false;
        }

        socket_opts |= O_NONBLOCK;
        fcntl(m_fd, F_SETFL, socket_opts);

        if (::connect(m_fd, &addr, addr_len) != 0) {
            return false;
        }

        FD_ZERO(&fdset);
        FD_SET(m_fd, &fdset);

        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        if (select(m_fd + 1, nullptr, &fdset, nullptr, &tv) == 1) {
            int so_error;
            socklen_t len = sizeof(so_error);

            getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &so_error, &len);
            if (so_error != 0) {
                // the connection was not successful
                return false;
            } else {
                // success, now make socket operations blocking again
                socket_opts &= ~O_NONBLOCK;
                if (fcntl(m_fd, F_SETFL, socket_opts) == -1) {
                    return false;
                }
            }
        }
    } else {
        // timeout not set, use blocking operation on socket
        if (::connect(m_fd, &addr, addr_len) != 0) {
            return false;
        }
    }

    m_connected = true;
    return m_connected;
}

bool Socket::connect(const std::string &ipaddr, const uint16_t &port, const int &timeout) {
    bool ret;
    struct addrinfo *address_info;
    struct addrinfo address_info_hints = {};

    address_info_hints.ai_family = AF_UNSPEC;
    address_info_hints.ai_socktype = SOCK_STREAM;
    address_info_hints.ai_protocol = 0;
    address_info_hints.ai_flags = AI_ADDRCONFIG;

    // get target address in network order
    if (getaddrinfo(ipaddr.c_str(), std::to_string(port).c_str(), &address_info_hints,
                    &address_info) != 0) {
        return false;
    }

    ret = connect(*address_info->ai_addr, timeout);
    freeaddrinfo(address_info);
    return ret;
}

void Socket::shutdown(const int &how) {
    ::shutdown(m_fd, how);
}

void Socket::close() {
    ::close(m_fd);
}

int Socket::poll(const int &timeout) const {
    struct pollfd poll_fds[1];

    // set up polling on socket
    poll_fds[0].fd = m_fd;
    poll_fds[0].events = POLLIN;

    // poll for <timeout>, then exit
    return ::poll(poll_fds, 1, timeout);
}

bool Socket::get_opt(const int &level, const int &opt_name, void *opt_val,
                     socklen_t *opt_len) const {
    return getsockopt(m_fd, level, opt_name, opt_val, opt_len) == 0;
}

bool Socket::set_opt(const int &level, const int &opt_name, const void *opt_val,
                     const socklen_t &opt_len) {
    bool success;

    success = setsockopt(m_fd, level, opt_name, opt_val, opt_len) == 0;
    if (success) {
        m_socket_opts.push_back(opt_name);
    }

    return success;
}

long Socket::get_timeout() {
    struct timeval timeout;
    socklen_t timeval_len = sizeof(struct timeval);

    if (!get_opt(SOL_SOCKET, SO_RCVTIMEO, &timeout, &timeval_len)) {
        return -1;
    }

    return timeout.tv_sec * 1000000 + timeout.tv_usec;
}

bool Socket::set_timeout(const int &us) {
    struct timeval timeout;
    time_t secs = us / 1000000;
    suseconds_t usecs = (us * 1000000 - secs) / 1000000;

    timeout.tv_sec = secs;
    timeout.tv_usec = usecs;

    return set_opt(SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

ssize_t Socket::receive(void *buf, const size_t &data_len_max, const int &flags) {
    ssize_t ret;

    ret = recv(m_fd, buf, data_len_max, flags);
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

ssize_t Socket::receive_msg(struct msghdr *msg, const int &flags) {
    ssize_t ret;

    ret = recvmsg(m_fd, msg, flags);
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

ssize_t Socket::send(const void *buf, const size_t &data_len, const int &flags) {
    ssize_t ret;

    ret = ::send(m_fd, buf, data_len, flags);
    if (ret == -1) {
        m_state = STATE_ERROR;
    }

    m_state = STATE_OK;
    return ret;
}

ssize_t Socket::send_msg(struct msghdr *msg, const int &flags) {
    ssize_t ret;

    ret = sendmsg(m_fd, msg, flags);
    if (ret == -1) {
        m_state = STATE_ERROR;
    }

    m_state = STATE_OK;
    return ret;
}
