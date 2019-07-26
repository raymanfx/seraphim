/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <poll.h>

#include "tcp_server.h"

using namespace sph::backend;

TCPServer::TCPServer(const int &domain) : m_transport(domain) {
    m_init = false;
    m_running = false;
}

TCPServer::~TCPServer() {
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

bool TCPServer::init(const uint16_t &port) {
    m_init = m_transport.bind(port);
    return m_init;
}

bool TCPServer::run() {
    if (!m_init) {
        return false;
    }

    if (!m_transport.listen(1)) {
        return false;
    }

    m_running = true;
    m_thread = std::thread([&]() {
        std::vector<int> client_fds;
        std::vector<struct pollfd> poll_fds;

        while (m_running) {
            poll_fds.resize(client_fds.size() + 1);

            poll_fds[0].fd = m_transport.stream().socket().fd();
            poll_fds[0].events = POLLIN;
            for (size_t i = 0; i < client_fds.size(); i++) {
                poll_fds[i + 1].fd = client_fds[i];
                poll_fds[i + 1].events = POLLIN;
            }

            if (::poll(poll_fds.data(), poll_fds.size() + 1, 1000) <= 0) {
                continue;
            }

            // check which client got input
            if (poll_fds[0].revents == POLLIN) {
                // add a new client
                int client = m_transport.accept(nullptr, nullptr);
                if (client == -1) {
                    continue;
                }
                client_fds.push_back(client);
            }

            for (size_t i = 0; i < client_fds.size(); i++) {
                if (poll_fds[i + 1].revents != POLLIN) {
                    continue;
                }

                // get data from client
                if (!m_transport.recv(poll_fds[i + 1].fd, m_msg)) {
                    if (m_transport.client_disconnected()) {
                        client_fds.erase(client_fds.begin() + i);
                        continue;
                    }

                    // error ..
                    continue;
                }

                if (m_handler) {
                    m_handler(m_msg);
                }

                m_transport.send(poll_fds[i + 1].fd, m_msg);
            }
        }
    });

    return true;
}

void TCPServer::terminate() {
    m_running = false;
    m_transport.set_timeout(1);
    if (m_thread.joinable()) {
        m_thread.join();
    }
}
