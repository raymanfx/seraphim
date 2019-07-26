/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IPC_NET_SOCKET_H
#define SPH_IPC_NET_SOCKET_H

#include <arpa/inet.h>
#include <cstdint>
#include <string>
#include <vector>

namespace sph {
namespace ipc {
namespace net {

class Socket {
public:
    Socket(const int &domain, const int &type, const int &protocol);
    ~Socket();

    int fd() const { return m_fd; }

    enum state_t { STATE_ERROR = -1, STATE_OK = 0, STATE_TIMEOUT };

    state_t state() const { return m_state; }

    /* reset socket fd and state */
    void reset(const bool &keep_opts = false);

    /* bind with address structure */
    bool bind(const struct sockaddr &addr);
    bool bind(const uint16_t &port);

    /* connect to a server */
    bool connect(const struct sockaddr &addr, const int &timeout = 0);
    bool connect(const std::string &ipaddr, const uint16_t &port, const int &timeout = 0);

    /* shutdown and close the socket (cleanup) */
    void shutdown(const int &how);
    void close();

    /* poll for I/O activity */
    int poll(const int &timeout) const;

    /* manipulate socket options */
    bool get_opt(const int &level, const int &opt_name, void *opt_val, socklen_t *opt_len) const;
    bool set_opt(const int &level, const int &opt_name, const void *opt_val,
                 const socklen_t &opt_len);

    /* convenience API */
    long get_timeout();
    bool set_timeout(const int &us);

    /* RX and TX */
    ssize_t receive(void *buf, const size_t &data_len_max, const int &flags = 0);
    ssize_t receive_msg(struct msghdr *msg, const int &flags = 0);
    ssize_t send(const void *buf, const size_t &data_len, const int &flags = 0);
    ssize_t send_msg(struct msghdr *msg, const int &flags = 0);

protected:
    /* socket fd */
    int m_fd;

    /* socket state */
    state_t m_state;
    bool m_connected;
    bool m_bound;

private:
    /* socket attributes */
    int m_domain;
    int m_type;
    int m_protocol;

    /* socket opts, set by user */
    std::vector<int> m_socket_opts;
};

} // namespace net
} // namespace ipc
} // namespace sph

#endif // SPH_IPC_NET_SOCKET_H
