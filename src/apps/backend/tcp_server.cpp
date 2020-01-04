/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <poll.h>
#include <seraphim/except.hpp>
#include <seraphim/ipc/except.hpp>
#include <seraphim/ipc/tcp_transport.hpp>

#include "tcp_server.hpp"

using namespace sph;
using namespace sph::backend;
using namespace sph::ipc;

TCPServer::TCPServer(std::shared_ptr<TCPTransport> ptr) : m_transport(ptr), m_running(false) {}

TCPServer::~TCPServer() {
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

bool TCPServer::run() {
    m_transport->synchronized<TCPTransport>()->listen(1);

    m_running = true;
    m_thread = std::thread([&]() {
        std::vector<int> client_fds;
        std::vector<struct pollfd> poll_fds;

        while (m_running) {
            poll_fds.resize(client_fds.size() + 1);

            poll_fds[0].fd = m_transport->synchronized<TCPTransport>()->socket().fd();
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
                int client = m_transport->synchronized<TCPTransport>()->accept(nullptr, nullptr);
                if (client == -1) {
                    continue;
                }
                emit_event(EVENT_CLIENT_CONNECTED, nullptr);
                client_fds.push_back(client);
                continue;
            }

            for (size_t i = 0; i < client_fds.size(); i++) {
                if (poll_fds[i + 1].revents != POLLIN) {
                    continue;
                }

                // get data from client
                try {
                    m_transport->synchronized<TCPTransport>()->receive(client_fds[i], m_msg);
                    emit_event(EVENT_MESSAGE_INBOUND, &m_msg);
                    handle_message(m_msg);
                    emit_event(EVENT_MESSAGE_OUTBOUND, &m_msg);
                    m_transport->synchronized<TCPTransport>()->send(client_fds[i], m_msg);
                } catch (const TimeoutException &) {
                    // ignore
                    continue;
                } catch (const PeerDisconnectedException &) {
                    emit_event(EVENT_CLIENT_DISCONNECTED, nullptr);
                    client_fds.erase(client_fds.begin() + i);
                    continue;
                } catch (const RuntimeException &e) {
                    std::cout << "[ERROR] TCPServer: " << e.what() << std::endl;
                }
            }
        }
    });

    return true;
}

void TCPServer::terminate() {
    m_running = false;
    m_transport->set_rx_timeout(1);
    m_transport->set_tx_timeout(1);
    if (m_thread.joinable()) {
        m_thread.join();
    }
}
