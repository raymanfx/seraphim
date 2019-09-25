/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <seraphim/core/except.h>

#include "shm_server.h"

using namespace sph::core;
using namespace sph::backend;

SharedMemoryServer::SharedMemoryServer() {
    m_init = false;
    m_running = false;
}

SharedMemoryServer::~SharedMemoryServer() {
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

bool SharedMemoryServer::init(const std::string &uri) {
    if (uri.find("shm://", 0, strlen("shm://")) == std::string::npos) {
        return false;
    }

    m_transport = sph::ipc::TransportFactory::Instance().create(uri);
    m_init = m_transport != nullptr;
    return m_init;
}

bool SharedMemoryServer::run() {
    if (!m_init) {
        return false;
    }

    m_running = true;
    m_thread = std::thread([&]() {
        while (m_running) {
            try {
                m_transport->receive(m_msg);
                handle_event(EVENT_MESSAGE_INCOMING, &m_msg);
                m_transport->send(m_msg);
            } catch (TimeoutException) {
                // ignore
                continue;
            } catch (RuntimeException &e) {
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

void SharedMemoryServer::register_event_handler(const event_t &mask,
                                                const event_handler_t handler) {
    m_event_handlers.push_back(std::make_pair(mask, handler));
}

void SharedMemoryServer::handle_event(const event_t &event, void *data) {
    for (const auto &handler : m_event_handlers) {
        if (handler.first & event) {
            handler.second(data);
        }
    }
}
