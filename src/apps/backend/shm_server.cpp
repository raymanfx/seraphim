/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include "shm_server.h"

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

bool SharedMemoryServer::init(const std::string &shm_name, const long &shm_size) {
    m_init = m_transport.create(shm_name, shm_size);
    return m_init;
}

bool SharedMemoryServer::run() {
    if (!m_init) {
        return false;
    }

    m_running = true;
    m_thread = std::thread([&]() {
        while (m_running) {
            if (!m_transport.recv(m_msg)) {
                continue;
            }

            if (m_handler) {
                m_handler(m_msg);
            }

            m_transport.send(m_msg);
        }
    });

    return true;
}

void SharedMemoryServer::terminate() {
    m_running = false;
    m_transport.set_timeout(1);
    if (m_thread.joinable()) {
        m_thread.join();
    }
}
