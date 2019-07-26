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

#include "seraphim/ipc/tcp_transport.h"

using namespace sph::ipc;

TCPTransport::TCPTransport(const int &domain) : m_stream(domain) {
    m_client_disconnected = false;
}

bool TCPTransport::recv(Seraphim::Message &msg) {
    if (m_stream.receive(m_rx_buffer) == -1) {
        return false;
    }

    if (!msg.ParseFromArray(m_rx_buffer.data(), static_cast<int>(m_rx_buffer.size()))) {
        return false;
    }

    return true;
}

bool TCPTransport::send(const Seraphim::Message &msg) {
    if (m_tx_buffer.size() != msg.ByteSizeLong()) {
        m_tx_buffer.resize(msg.ByteSizeLong());
    }

    if (!msg.SerializeToArray(m_tx_buffer.data(), msg.ByteSize())) {
        return false;
    }

    if (!m_stream.send(m_tx_buffer.data(), m_tx_buffer.size())) {
        return false;
    }

    return true;
}

bool TCPTransport::recv(const int &fd, Seraphim::Message &msg) {
    ssize_t read;

    m_client_disconnected = false;
    read = m_stream.receive(fd, m_rx_buffer);
    if (read == -1) {
        return false;
    } else if (read == 0) {
        m_client_disconnected = true;
        return false;
    }

    if (!msg.ParseFromArray(m_rx_buffer.data(), static_cast<int>(m_rx_buffer.size()))) {
        return false;
    }

    return true;
}

bool TCPTransport::send(const int &fd, const Seraphim::Message &msg) {
    if (m_tx_buffer.size() != msg.ByteSizeLong()) {
        m_tx_buffer.resize(msg.ByteSizeLong());
    }

    if (!msg.SerializeToArray(m_tx_buffer.data(), msg.ByteSize())) {
        return false;
    }

    if (!m_stream.send(fd, m_tx_buffer.data(), m_tx_buffer.size())) {
        return false;
    }

    return true;
}