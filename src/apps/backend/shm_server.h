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

class SharedMemoryServer : public sph::backend::Server {
public:
    SharedMemoryServer(std::shared_ptr<sph::ipc::SharedMemoryTransport> ptr);
    ~SharedMemoryServer() override;

    bool run() override;
    void terminate() override;

private:
    std::shared_ptr<sph::ipc::SharedMemoryTransport> m_transport;

    std::thread m_thread;
    std::atomic<bool> m_running;
    Seraphim::Message m_msg;
};

} // namespace backend
} // namespace sph

#endif // SPH_SHARED_MEMORY_SERVER_H
