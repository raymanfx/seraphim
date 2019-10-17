/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_TCP_SERVER_H
#define SPH_TCP_SERVER_H

#include <atomic>
#include <seraphim/ipc/transport_factory.h>
#include <thread>

#include "server.h"

namespace sph {
namespace backend {

class TCPServer : public sph::backend::Server {
public:
    TCPServer();
    ~TCPServer() override;

    bool init(const std::string &uri) override;
    bool run() override;
    void terminate() override;

private:
    std::unique_ptr<sph::ipc::Transport> m_transport;
    bool m_init;

    std::thread m_thread;
    std::atomic<bool> m_running;
    Seraphim::Message m_msg;
};

} // namespace backend
} // namespace sph

#endif // SPH_TCP_SERVER_H
