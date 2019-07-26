/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "seraphim/ipc/net/tcp_stream.h"

using namespace sph::ipc::net;

TCPStream::TCPStream(const int &domain) : m_socket(domain) {

    // allow address reuse by default
    int enable = 1;
    m_socket.set_opt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(enable));

    m_packet_size = 1500;
}

ssize_t TCPStream::receive(std::vector<uint8_t> &buf) {
    ssize_t read = 0;
    ssize_t packet;
    MessageHeader msghdr = {};

    read = m_socket.receive(&msghdr, sizeof(msghdr));
    if (read <= 0) {
        return read;
    }

    if (buf.size() != msghdr.size) {
        buf.resize(msghdr.size);
    }

    read = 0;
    while (static_cast<uint64_t>(read) < msghdr.size) {
        packet = m_socket.receive(buf.data() + read, msghdr.size - read, 0);
        if (packet <= 0) {
            return packet;
        }
        read += packet;
    }

    return read;
}

ssize_t TCPStream::receive(const int &fd, std::vector<uint8_t> &buf) {
    ssize_t read = 0;
    ssize_t packet;
    MessageHeader msghdr = {};

    read = m_socket.receive(fd, &msghdr, sizeof(msghdr));
    if (read <= 0) {
        return read;
    }

    if (buf.size() != msghdr.size) {
        buf.resize(msghdr.size);
    }

    read = 0;
    while (static_cast<uint64_t>(read) < msghdr.size) {
        packet = m_socket.receive(fd, buf.data() + read, msghdr.size - read, 0);
        if (packet <= 0) {
            return packet;
        }
        read += packet;
    }

    return read;
}

ssize_t TCPStream::send(const void *buf, const size_t &data_len) {
    ssize_t sent = 0;
    size_t packet_size;
    size_t data_left = data_len;
    MessageHeader msghdr = {};

    msghdr.size = data_len;
    sent = m_socket.send(&msghdr, sizeof(msghdr), 0);
    if (sent != sizeof(msghdr)) {
        return sent;
    }

    while (data_left > 0) {
        packet_size = data_left > m_packet_size ? m_packet_size : data_left;
        sent = m_socket.send(reinterpret_cast<const char *>(buf) + (data_len - data_left),
                             packet_size, 0);
        if (static_cast<size_t>(sent) != packet_size) {
            return sent;
        }
        data_left -= static_cast<size_t>(sent);
    }

    return sent;
}

ssize_t TCPStream::send(const int &fd, const void *buf, const size_t &data_len) {
    ssize_t sent = 0;
    size_t packet_size;
    size_t data_left = data_len;
    MessageHeader msghdr = {};

    msghdr.size = data_len;
    sent = m_socket.send(fd, &msghdr, sizeof(msghdr), 0);
    if (sent != sizeof(msghdr)) {
        return sent;
    }

    while (data_left > 0) {
        packet_size = data_left > m_packet_size ? m_packet_size : data_left;
        sent = m_socket.send(fd, reinterpret_cast<const char *>(buf) + (data_len - data_left),
                             packet_size, 0);
        if (static_cast<size_t>(sent) != packet_size) {
            return sent;
        }
        data_left -= static_cast<size_t>(sent);
    }

    return sent;
}
