/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <poll.h>
#include <seraphim/except.h>
#include <seraphim/ipc/except.h>
#include <seraphim/ipc/tcp_transport.h>

#include "tcp_server.h"

using namespace sph;
using namespace sph::ipc;
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

bool TCPServer::init(const std::string &uri) {
    if (uri.find("tcp://", 0, strlen("tcp://")) == std::string::npos) {
        return false;
    }

    m_transport = sph::ipc::TransportFactory::Instance().create(uri);
    m_init = m_transport != nullptr;
    return m_init;
}

bool TCPServer::run() {
    sph::ipc::TCPTransport *tcp = static_cast<sph::ipc::TCPTransport *>(m_transport.get());

    if (!m_init) {
        return false;
    }

    tcp->listen(1);

    m_running = true;
    m_thread = std::thread([&]() {
        sph::ipc::TCPTransport *tcp = static_cast<sph::ipc::TCPTransport *>(m_transport.get());
        std::vector<int> client_fds;
        std::vector<struct pollfd> poll_fds;

        while (m_running) {
            poll_fds.resize(client_fds.size() + 1);

            poll_fds[0].fd = tcp->socket().fd();
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
                    tcp->receive(client_fds[i], m_msg);
                    emit_event(EVENT_MESSAGE_INBOUND, &m_msg);
                    handle_message(m_msg);
                    emit_event(EVENT_MESSAGE_OUTBOUND, &m_msg);
                    tcp->send(client_fds[i], m_msg);
                } catch (TimeoutException) {
                    // ignore
                    continue;
                } catch (PeerDisconnectedException) {
                    emit_event(EVENT_CLIENT_DISCONNECTED, nullptr);
                    client_fds.erase(client_fds.begin() + i);
                    continue;
                } catch (RuntimeException &e) {
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
