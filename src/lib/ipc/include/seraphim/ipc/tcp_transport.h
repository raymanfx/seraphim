/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IPC_TCP_TRANSPORT_H
#define SPH_IPC_TCP_TRANSPORT_H

#include "net/socket.h"
#include "net/tcp_socket.h"
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
     * @brief Message header.
     *
     * Used to bring the concept of message boundaries to TCP.
     * This way, a sender may send a message and the client may receive that message without any
     * previous knowledge about its structure or size.
     */
    struct MessageHeader {
        /// Transmission size
        uint64_t size;
    } __attribute__((packed));

    /**
     * @brief TCP message transport.
     */
    explicit TCPTransport(net::Socket::Family family) : m_socket(family) {}

    /**
     * @brief Get the stream associated with this transport.
     * @return The TCP stream instance created by this instance.
     */
    sph::ipc::net::TCPSocket &socket() { return m_socket; }

    /**
     * @brief Bind to a port.
     *        Throws sph::RuntimeException in case of errors.
     * @param port The port number, must be a value between 0 and 65535.
     * @return True on success, false otherwise.
     */
    bool bind(uint16_t port) { return m_socket.bind(port); }

    /**
     * @brief Connect to another socket.
     *        Whether TX operations are legal without an active connection depends on the socket
     *        type and protocol. E.g. TCP sockets require a connection, but UDP datagram sockets do
     *        not.
     *        Throws sph::RuntimeException in case of errors.
     * @param port The port number, must be a value between 0 and 65535.
     * @return True on success, false otherwise.
     */
    bool connect(const std::string &ipaddr, uint16_t port, int timeout = 0) {
        return m_socket.connect(ipaddr, port, timeout);
    }

    /**
     * @brief Listen for incoming client connections (must be bound to a port already).
     *        Throws sph::RuntimeException when the OS socket op fails.
     * @param backlog Number of clients that can simultaneously be connected.
     */
    void listen(int backlog) { m_socket.listen(backlog); }

    /**
     * @brief Accept a client connection.
     * @param addr Pointer to an address struct where the client address is stored.
     * @param addrlen Pointer to length of the address struct.
     * @return File descriptor for the new connection on success, -1 otherwise.
     */
    int accept(struct sockaddr *addr, socklen_t *addrlen) { return m_socket.accept(addr, addrlen); }

    void set_rx_timeout(int ms) override { m_socket.set_rx_timeout(ms * 1000); }
    void set_tx_timeout(int ms) override { m_socket.set_tx_timeout(ms * 1000); }

    void receive(Seraphim::Message &msg) override;
    void send(const Seraphim::Message &msg) override;

    /**
     * @brief Receive a message from a client.
     *        Throws sph::RuntimeException in case of errors.
     *        Throws sph::TimeoutException in case of timeouts.
     * @param fd File descriptor of the client connection.
     * @param msg The message.
     */
    void receive(int fd, Seraphim::Message &msg);

    /**
     * @brief Send a message to a client.
     *        Throws sph::RuntimeException in case of errors.
     *        Throws sph::TimeoutException in case of timeouts.
     * @param fd File descriptor of the client connection.
     * @param msg The message.
     */
    void send(int fd, const Seraphim::Message &msg);

private:
    /// TCP socket OS implementation
    sph::ipc::net::TCPSocket m_socket;

    /// RX buffer used for storing deserialized, inbound messages
    std::vector<uint8_t> m_rx_buffer;
    /// TX buffer used for storing serialized, outbound messages
    std::vector<uint8_t> m_tx_buffer;
};

} // namespace ipc
} // namespace sph

#endif // SPH_IPC_TCP_TRANSPORT_H
