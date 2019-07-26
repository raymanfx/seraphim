/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IPC_NET_TCP_STREAM_H
#define SPH_IPC_NET_TCP_STREAM_H

#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include "tcp_socket.h"

namespace sph {
namespace ipc {
namespace net {

class TCPStream {
public:
    explicit TCPStream(const int &domain);
    ~TCPStream() = default;

    struct MessageHeader {
        // transmission size
        uint64_t size;
    } __attribute__((packed));

    sph::ipc::net::TCPSocket &socket() { return m_socket; }

    ssize_t receive(std::vector<uint8_t> &buf);
    ssize_t receive(const int &fd, std::vector<uint8_t> &buf);
    ssize_t send(const void *buf, const size_t &data_len);
    ssize_t send(const int &fd, const void *buf, const size_t &data_len);

protected:
    sph::ipc::net::TCPSocket m_socket;
    uint16_t m_packet_size;
};

} // namespace net
} // namespace ipc
} // namespace sph

#endif // SPH_IPC_NET_TCP_STREAM_H
