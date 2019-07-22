/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <poll.h>
#include <seraphim/ipc/tcp_transport.h>

#include "tcp_server.h"

using namespace sph::backend;

TCPServer::TCPServer() {
    m_init = false;
    m_running = false;
}

TCPServer::~TCPServer() {
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

bool TCPServer::init(const std::string &address, const uint16_t &port) {
    std::string uri = "tcp://" + address + ":" + std::to_string(port);
    m_transport = sph::ipc::TransportFactory::Instance().create(uri);
    m_init = m_transport != nullptr;
    return m_init;
}

bool TCPServer::run() {
    sph::ipc::TCPTransport *tcp = static_cast<sph::ipc::TCPTransport *>(m_transport.get());

    if (!m_init) {
        return false;
    }

    if (!tcp->listen(1)) {
        return false;
    }

    m_running = true;
    m_thread = std::thread([&]() {
        sph::ipc::TCPTransport *tcp = static_cast<sph::ipc::TCPTransport *>(m_transport.get());
        std::vector<int> client_fds;
        std::vector<struct pollfd> poll_fds;

        while (m_running) {
            poll_fds.resize(client_fds.size() + 1);

            poll_fds[0].fd = tcp->stream().socket().fd();
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
                int client = tcp->accept(nullptr, nullptr);
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
                if (!tcp->recv(poll_fds[i + 1].fd, m_msg)) {
                    if (tcp->client_disconnected()) {
                        client_fds.erase(client_fds.begin() + i);
                        continue;
                    }

                    // error ..
                    continue;
                }

                handle_event(EVENT_MESSAGE_INCOMING, &m_msg);
                tcp->send(poll_fds[i + 1].fd, m_msg);
            }
        }
    });

    return true;
}

void TCPServer::terminate() {
    m_running = false;
    m_transport->set_timeout(1);
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void TCPServer::register_event_handler(const event_t &mask, const event_handler_t handler) {
    m_event_handlers.push_back(std::make_pair(mask, handler));
}

void TCPServer::handle_event(const event_t &event, void *data) {
    for (const auto &handler : m_event_handlers) {
        if (handler.first & event) {
            handler.second(data);
        }
    }
}
