/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_TCP_SERVER_HPP
#define SPH_TCP_SERVER_HPP

#include <atomic>
#include <seraphim/ipc/tcp_transport.hpp>
#include <thread>

#include "server.hpp"

namespace sph {
namespace backend {

class TCPServer : public sph::backend::Server {
public:
    TCPServer(std::shared_ptr<sph::ipc::TCPTransport> ptr);
    ~TCPServer() override;

    bool run() override;
    void terminate() override;

private:
    std::shared_ptr<sph::ipc::TCPTransport> m_transport;

    std::thread m_thread;
    std::atomic<bool> m_running;
    Seraphim::Message m_msg;
};

} // namespace backend
} // namespace sph

#endif // SPH_TCP_SERVER_HPP
