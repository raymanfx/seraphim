/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_TCP_SERVER_H
#define SPH_TCP_SERVER_H

#include <atomic>
#include <seraphim/ipc/tcp_transport.h>
#include <thread>

#include "server.h"

namespace sph {
namespace backend {

class TCPServer : public sph::backend::IServer {
public:
    explicit TCPServer(const int &domain);
    ~TCPServer() override;

    bool run() override;
    void terminate() override;
    void register_event_handler(const event_t &mask, const event_handler_t handler) override;

    bool init(const uint16_t &port);
    void handle_event(const event_t &event, void *data);

private:
    std::string m_shm_name;
    long m_shm_size;

    sph::ipc::TCPTransport m_transport;
    bool m_init;

    std::thread m_thread;
    std::atomic<bool> m_running;
    Seraphim::Message m_msg;

    std::vector<std::pair<IServer::event_t, IServer::event_handler_t>> m_event_handlers;
};

} // namespace backend
} // namespace sph

#endif // SPH_TCP_SERVER_H
