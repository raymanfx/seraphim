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
    SharedMemoryServer();
    ~SharedMemoryServer() override;

    bool run() override;
    void terminate() override;

    bool init(const std::string &shm_name, const long &shm_size);

private:
    std::string m_shm_name;
    long m_shm_size;

    sph::ipc::SharedMemoryTransport m_transport;
    bool m_init;

    std::thread m_thread;
    std::atomic<bool> m_running;
    Seraphim::Message m_msg;
};

} // namespace backend
} // namespace sph

#endif // SPH_SHARED_MEMORY_SERVER_H
