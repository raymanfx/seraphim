/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IPC_NET_SOCKET_H
#define SPH_IPC_NET_SOCKET_H

#include <arpa/inet.h>
#include <cstdint>
#include <string.h>
#include <string>
#include <vector>

namespace sph {
namespace ipc {
namespace net {

/**
 * @brief Socket abstraction.
 *
 * Multiple socket types are supported, depending on your operating system.
 * Basic socket functionality includes state handling (binding, closing, etc) and basic I/O for
 * sending and receiving arbitrary byte data.
 */
class Socket {
public:
    /**
     * @brief Socket family.
     *        At the moment, only internet sockets are supported.
     */
    enum class Family {
        /// IPv4
        INET,
        /// IPv6
        INET6
    };

    /**
     * @brief Socket type.
     *        TODO: Add UNIX sockets and friends.
     */
    enum class Type {
        /// Stream (connection)
        STREAM,
        /// Datagram (connection-less)
        DATAGRAM
    };

    /**
     * @brief Socket protocol.
     *        Used during socket creation.
     */
    enum class Protocol {
        /// Transmission Control Protocol
        TCP,
        /// User Datagram Protocol
        UDP
    };

    /**
     * @brief Socket shutdown operation rules.
     *        Pending opeations will be cancelled depending on what value you choose here.
     *        The socket destructor will always shutdown the socket using Shutdown::BOTH.
     */
    enum class Shutdown {
        /// Further read operations are not allowed
        READ,
        /// Further write operations are not allowed
        WRITE,
        /// Any further I/O operations are not allowed.
        BOTH
    };

    /**
     * @brief OS specifc socket implementation.
     *        Throws sph::InvalidArgumentException in case of unsupported family, type or
     *        protocol arguments.
     * @param family See @ref Family.
     * @param type See @ref Type.
     * @param protocol See @ref Protocol.
     */
    Socket(Family family, Type type, Protocol protocol);

    Socket(const Socket &) = delete;
    Socket(Socket &&) noexcept = default;

    Socket &operator=(const Socket &) = delete;
    Socket &operator=(const Socket &&) = delete;

    /**
     * @brief Cleanup the socket, interrupting all of its I/O ops and closing it.
     */
    ~Socket();

    /**
     * @brief Reset the socket, effectively creating a new underlying OS socket instance.
     *        Throws sph::RuntimeException when the OS socket creation fails.
     * @param keep_opts Whether to keep any options set on the current socket.
     */
    void reset(bool keep_opts = false);

    const char *err_str() const noexcept {
        static char buffer[1024];
        return strerror_r(err(), buffer, 1024);
    }

    /**
     * @brief File descriptor of the socket.
     * @return The fd as integer.
     */
    int fd() const { return m_fd; }

    /**
     * @brief Bind to a port.
     *        Throws sph::RuntimeException in case of errors.
     * @param port The port number, must be a value between 0 and 65535.
     * @return True on success, false otherwise.
     */
    bool bind(uint16_t port);

    /**
     * @brief Connect to another socket.
     *        Whether TX operations are legal without an active connection depends on the socket
     *        type and protocol. E.g. TCP sockets require a connection, but UDP datagram sockets do
     *        not.
     *        Throws sph::RuntimeException in case of errors.
     * @param port The port number, must be a value between 0 and 65535.
     * @return True on success, false otherwise.
     */
    bool connect(const std::string &ipaddr, uint16_t port, int timeout = 0);

    /**
     * @brief Shutdown the socket, cancelling any ongoing read/write operations.
     *        Throws sph::RuntimeException when the OS socket shutdown fails.
     * @param how See @ref Shutdown.
     */
    void shutdown(Shutdown how = Shutdown::BOTH);

    /**
     * @brief Poll the socket for I/O activity such as incoming data.
     *        Throws sph::RuntimeException when the OS socket poll fails.
     * @param timeout Timeout in milliseconds.
     * @return True if there is activity on the socket, false otherwise.
     */
    bool poll(int timeout) const;

    /**
     * @brief Set a timeout for read operations on the socket.
     *        Throws sph::RuntimeException when the OS socket op fails.
     *        @see set_opt().
     * @param us Timeout in microseconds.
     */
    void set_rx_timeout(long us);

    /**
     * @brief Set a timeout for write operations on the socket.
     *        Throws sph::RuntimeException when the OS socket op fails.
     *        @see set_opt().
     * @param us Timeout in microseconds.
     */
    void set_tx_timeout(long us);

    /**
     * @brief Receive an arbitrary number of bytes.
     *        Throws sph::RuntimeException when the OS socket connection fails.
     *        Throws sph::TimeoutException when the OS socket connection times out.
     * @param buf Output buffer.
     * @param max_len Maximum number of bytes to receive.
     * @param flags OS socket flags.
     * @return Number of bytes received.
     */
    ssize_t receive(void *buf, size_t max_len, int flags = 0);

    /**
     * @brief Transmit a number of bytes.
     *        Throws sph::RuntimeException when the OS socket connection fails.
     *        Throws sph::TimeoutException when the OS socket connection times out.
     * @param buf Input buffer.
     * @param len Number of bytes to send. Must not be larger than the input buffer length.
     * @param flags OS socket flags.
     * @return Number of bytes sent.
     */
    ssize_t send(const void *buf, size_t len, int flags = 0);

    /**
     * @brief Gathering (vectored receival) of data.
     *        Throws sph::RuntimeException when the OS socket connection fails.
     *        Throws sph::TimeoutException when the OS socket connection times out.
     * @param msg Message structure.
     * @param flags OS socket flags.
     * @return Number of bytes received.
     */
    ssize_t receive_msg(struct msghdr *msg, int flags = 0);

    /**
     * @brief Scattering (vectored transmission) of data.
     *        Throws sph::RuntimeException when the OS socket connection fails.
     *        Throws sph::TimeoutException when the OS socket connection times out.
     * @param msg Message structure.
     * @param flags OS socket flags.
     * @return Number of bytes sent.
     */
    ssize_t send_msg(struct msghdr *msg, int flags = 0);

protected:
    /**
     * @brief Retrieve OS specific error codes.
     * @return The last error code.
     */
    int err() const noexcept { return errno; }

    /**
     * @brief Retrieve the value for a given socket option.
     *        Should only be used to implement higher abstraction levels.
     *        Throws sph::RuntimeException when the OS socket op fails.
     * @param level OS specific level, such as SOL_SOCKET or TCP.
     * @param opt_name Name of the option, represented as number.
     * @param opt_val Output value buffer.
     * @param opt_len Length of the output buffer.
     */
    void get_opt(int level, int opt_name, void *opt_val, socklen_t *opt_len) const;

    /**
     * @brief Set the value for a given socket option.
     *        Should only be used to implement higher abstraction levels such as @ref
     *        set_tx_timeout().
     *        Throws sph::RuntimeException when the OS socket op fails.
     * @param level OS specific level, such as SOL_SOCKET or TCP.
     * @param opt_name Name of the option, represented as number.
     * @param opt_val Output value buffer.
     * @param opt_len Length of the output buffer.
     */
    void set_opt(int level, int opt_name, const void *opt_val, socklen_t opt_len);

    /// OS socket file descriptor
    int m_fd = -1;

    /// Whether the socket is connected
    bool m_connected = false;
    /// Whether the socket is bound
    bool m_bound = false;

private:
    /* socket attributes */
    int m_family;
    int m_type;
    int m_protocol;

    /* socket opts, set by user */
    std::vector<int> m_socket_opts;
};

} // namespace net
} // namespace ipc
} // namespace sph

#endif // SPH_IPC_NET_SOCKET_H
