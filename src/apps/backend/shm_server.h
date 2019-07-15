/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_SHARED_MEMORY_SERVER_H
#define SPH_SHARED_MEMORY_SERVER_H

#include <atomic>
#include <seraphim/ipc/shm_transport.h>
#include <thread>

#include "server.h"

namespace sph {
namespace backend {

class SharedMemoryServer : public sph::backend::IServer {
public:
    SharedMemoryServer();
    ~SharedMemoryServer() override;

    bool run() override;
    void terminate() override;
    void register_event_handler(const event_t &mask, const event_handler_t handler) override;

    bool init(const std::string &shm_name, const long &shm_size);
    void handle_event(const event_t &event, void *data);

private:
    std::string m_shm_name;
    long m_shm_size;

    sph::ipc::SharedMemoryTransport m_transport;
    bool m_init;

    std::thread m_thread;
    std::atomic<bool> m_running;
    Seraphim::Message m_msg;

    std::vector<std::pair<IServer::event_t, IServer::event_handler_t>> m_event_handlers;
};

} // namespace backend
} // namespace sph

#endif // SPH_SHARED_MEMORY_SERVER_H
