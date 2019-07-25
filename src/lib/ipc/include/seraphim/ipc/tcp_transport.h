/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IPC_TCP_TRANSPORT_H
#define SPH_IPC_TCP_TRANSPORT_H

#include "net/tcp_stream.h"
#include "transport.h"

namespace sph {
namespace ipc {

/**
 * @brief TCP message transport.
 *
 * This class uses TCP to exchange messages between a server and a client. A single server can
 * handle multiple clients, but their requests will be processed serially.
 */
class TCPTransport : public ITransport {
public:
    /**
     * @brief TCP message transport.
     */
    explicit TCPTransport(const int &domain);

    /**
     * @brief Get the stream associated with this transport.
     * @return The TCP stream instance created by this instance.
     */
    sph::ipc::net::TCPStream &stream() { return m_stream; }

    /**
     * @brief Bind to a port on any network interface.
     * @param port The network port to bind on.
     * @return true on success, false otherwise.
     */
    bool bind(const uint16_t &port) { return m_stream.socket().bind(port); }

    /**
     * @brief Connect to a server on a given port.
     * @param ipaddr IP address of the server.
     * @param port Port of the server.
     * @param timeout Optional amount of milliseconds before aborting.
     * @return true on success, false otherwise.
     */
    bool connect(const std::string &ipaddr, const uint16_t &port, const int &timeout = 0) {
        return m_stream.socket().connect(ipaddr, port, timeout);
    }

    /**
     * @brief Listen for incoming client connections (must be bound to a port already).
     * @param backlog Number of clients that can simultaneously be connected.
     * @return true on success, false otherwise.
     */
    bool listen(const int &backlog) { return m_stream.socket().listen(backlog); }

    /**
     * @brief Accept a client connection.
     * @param addr Pointer to an address struct where the client address is stored.
     * @param addrlen Pointer to length of the address struct.
     * @return File descriptor for the new connection on success, -1 otherwise.
     */
    int accept(struct sockaddr *addr, socklen_t *addrlen) {
        return m_stream.socket().accept(addr, addrlen);
    }

    /**
     * @brief Check whether a client has disconnected.
     * This is useful when \ref recv returns false (when running as server) to check whether there
     * was an error or just a disconnect.
     * @return true if a client has disconnected, false otherwise.
     */
    bool client_disconnected() { return m_client_disconnected; }

    void set_timeout(const int &ms) override { m_stream.socket().set_timeout(ms * 1000); }

    IOResult recv(Seraphim::Message &msg) override;
    IOResult send(const Seraphim::Message &msg) override;

    /**
     * @brief Receive a message from a client.
     * @param fd File descriptor of the client connection.
     * @param msg The message.
     * @return true on success, false on error or timeout.
     */
    IOResult recv(const int &fd, Seraphim::Message &msg);

    /**
     * @brief Send a message to a client.
     * @param fd File descriptor of the client connection.
     * @param msg The message.
     * @return true on success, false on error or timeout.
     */
    IOResult send(const int &fd, const Seraphim::Message &msg);

private:
    /// TCP data stream
    sph::ipc::net::TCPStream m_stream;
    /// RX buffer used for storing deserialized, inbound messages
    std::vector<uint8_t> m_rx_buffer;
    /// TX buffer used for storing serialized, outbound messages
    std::vector<uint8_t> m_tx_buffer;
    /// Whether or not a client has just disconnected
    bool m_client_disconnected;
};

} // namespace ipc
} // namespace sph

#endif // SPH_IPC_TCP_TRANSPORT_H
