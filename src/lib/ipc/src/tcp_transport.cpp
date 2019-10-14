/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <chrono>
#include <cstring>
#include <poll.h>
#include <thread>
#include <unistd.h>

#include "seraphim/core/except.h"
#include "seraphim/ipc/tcp_transport.h"

using namespace sph;
using namespace sph::ipc;

void TCPTransport::receive(Seraphim::Message &msg) {
    MessageHeader msghdr = {};
    ssize_t read;

    read = m_socket.receive(&msghdr, sizeof(msghdr));
    if (read != sizeof(MessageHeader)) {
        SPH_THROW(RuntimeException, "Failed to receive message header");
    }

    if (m_rx_buffer.size() < msghdr.size) {
        m_rx_buffer.resize(msghdr.size);
    }

    size_t remaining = msghdr.size;
    do {
        // receive() will throw on error
        read = m_socket.receive(m_rx_buffer.data() + msghdr.size - remaining, msghdr.size);
        assert(read > 0);

        // calculate the remaining amount of bytes to be read
        remaining -= static_cast<size_t>(read);
    } while (remaining > 0);

    if (!msg.ParseFromArray(m_rx_buffer.data(), static_cast<int>(msghdr.size))) {
        SPH_THROW(RuntimeException, "Failed to deserialize message");
    }
}

void TCPTransport::send(const Seraphim::Message &msg) {
    MessageHeader msghdr = {};
    ssize_t sent;

    msghdr.size = msg.ByteSizeLong();
    sent = m_socket.send(&msghdr, sizeof(msghdr));
    if (static_cast<uint64_t>(sent) != sizeof(msghdr)) {
        SPH_THROW(RuntimeException, "Failed to send message header");
    }

    if (m_tx_buffer.size() < msg.ByteSizeLong()) {
        m_tx_buffer.resize(msg.ByteSizeLong());
    }

    if (!msg.SerializeToArray(m_tx_buffer.data(), msg.ByteSize())) {
        SPH_THROW(RuntimeException, "Failed to serialize message");
    }

    size_t remaining = msghdr.size;
    do {
        // send() will throw on error
        sent = m_socket.send(m_tx_buffer.data() + msghdr.size - remaining, remaining);
        assert(sent > 0);

        // calculate the remaining amount of bytes to be sent
        remaining -= static_cast<size_t>(sent);
    } while (remaining > 0);
}

void TCPTransport::receive(int fd, Seraphim::Message &msg) {
    MessageHeader msghdr = {};
    ssize_t read;

    read = m_socket.receive(fd, &msghdr, sizeof(msghdr));
    if (read != sizeof(MessageHeader)) {
        SPH_THROW(RuntimeException, "Failed to receive message header");
    }

    if (m_rx_buffer.size() < msghdr.size) {
        m_rx_buffer.resize(msghdr.size);
    }

    size_t remaining = msghdr.size;
    do {
        // receive() will throw on error
        read = m_socket.receive(fd, m_rx_buffer.data() + msghdr.size - remaining, msghdr.size);
        assert(read > 0);

        // calculate the remaining amount of bytes to be read
        remaining -= static_cast<size_t>(read);
    } while (remaining > 0);

    if (m_rx_buffer.size() == 0) {
        SPH_THROW(RuntimeException, "Peer disconnected");
    }

    if (!msg.ParseFromArray(m_rx_buffer.data(), static_cast<int>(msghdr.size))) {
        SPH_THROW(RuntimeException, "Failed to deserialize message");
    }
}

void TCPTransport::send(int fd, const Seraphim::Message &msg) {
    MessageHeader msghdr = {};
    ssize_t sent;

    msghdr.size = msg.ByteSizeLong();
    sent = m_socket.send(fd, &msghdr, sizeof(msghdr));
    if (static_cast<uint64_t>(sent) != sizeof(msghdr)) {
        SPH_THROW(RuntimeException, "Failed to send message header");
    }

    if (m_tx_buffer.size() < msg.ByteSizeLong()) {
        m_tx_buffer.resize(msg.ByteSizeLong());
    }

    if (!msg.SerializeToArray(m_tx_buffer.data(), msg.ByteSize())) {
        SPH_THROW(RuntimeException, "Failed to serialize message");
    }

    size_t remaining = msghdr.size;
    do {
        // send() will throw on error
        sent = m_socket.send(fd, m_tx_buffer.data() + msghdr.size - remaining, remaining);
        assert(sent > 0);

        // calculate the remaining amount of bytes to be sent
        remaining -= static_cast<size_t>(sent);
    } while (remaining > 0);
}
