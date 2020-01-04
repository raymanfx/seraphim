/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <seraphim/except.hpp>

#include "shm_server.hpp"

using namespace sph;
using namespace sph::backend;
using namespace sph::ipc;

SharedMemoryServer::SharedMemoryServer(std::shared_ptr<SharedMemoryTransport> ptr)
    : m_transport(ptr), m_running(false) {}

SharedMemoryServer::~SharedMemoryServer() {
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

bool SharedMemoryServer::run() {
    m_running = true;
    m_thread = std::thread([&]() {
        while (m_running) {
            try {
                m_transport->synchronized()->receive(m_msg);
                emit_event(EVENT_MESSAGE_INBOUND, &m_msg);
                handle_message(m_msg);
                emit_event(EVENT_MESSAGE_OUTBOUND, &m_msg);
                m_transport->synchronized()->send(m_msg);
            } catch (const TimeoutException &) {
                // ignore
                continue;
            } catch (const RuntimeException &e) {
                std::cout << "[ERROR] SharedMemoryServer: " << e.what() << std::endl;
            }
        }
    });

    return true;
}

void SharedMemoryServer::terminate() {
    m_running = false;
    m_transport->set_rx_timeout(1);
    m_transport->set_tx_timeout(1);
    if (m_thread.joinable()) {
        m_thread.join();
    }
}
