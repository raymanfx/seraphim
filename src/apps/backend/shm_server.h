/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_SHARED_MEMORY_SERVER_H
#define SPH_SHARED_MEMORY_SERVER_H

#include <atomic>
#include <seraphim/ipc/transport_factory.h>
#include <thread>

#include "server.h"

namespace sph {
namespace backend {

class SharedMemoryServer : public sph::backend::Server {
public:
    SharedMemoryServer();
    ~SharedMemoryServer() override;

    bool init(const std::string &uri) override;
    bool run() override;
    void terminate() override;
    void register_event_handler(const event_t &mask, const event_handler_t handler) override;
    void handle_event(const event_t &event, void *data);

private:
    std::unique_ptr<sph::ipc::Transport> m_transport;
    bool m_init;

    std::thread m_thread;
    std::atomic<bool> m_running;
    Seraphim::Message m_msg;

    std::vector<std::pair<Server::event_t, Server::event_handler_t>> m_event_handlers;
};

} // namespace backend
} // namespace sph

#endif // SPH_SHARED_MEMORY_SERVER_H
