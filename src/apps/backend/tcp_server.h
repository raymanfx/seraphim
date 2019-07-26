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

class TCPServer : public sph::backend::Server {
public:
    explicit TCPServer(const int &domain);
    ~TCPServer() override;

    bool run() override;
    void terminate() override;

    bool init(const uint16_t &port);

private:
    std::string m_shm_name;
    long m_shm_size;

    sph::ipc::TCPTransport m_transport;
    bool m_init;

    std::thread m_thread;
    std::atomic<bool> m_running;
    Seraphim::Message m_msg;
};

} // namespace backend
} // namespace sph

#endif // SPH_TCP_SERVER_H
