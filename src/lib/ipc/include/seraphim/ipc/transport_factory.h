/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IPC_TRANSPORT_FACTORY_H
#define SPH_IPC_TRANSPORT_FACTORY_H

#include "transport.h"

namespace sph {
namespace ipc {

/**
 * @brief Transparent IPC transport factory.
 *
 * Allows you to create arbitrary transports by specifying only a string.
 * At the moment, shared memory and tcp based transports are supported.
 *
 * Example: "shm:///seraphim" would create a transport which operates on the shared memory segment
 * in /seraphim (/dev/shm/seraphim on Linux).
 */
class TransportFactory {
public:
    /**
     * @brief Singleton class instance.
     * @return The single, static instance of this class.
     */
    static TransportFactory &Instance() {
        // Guaranteed to be destroyed, instantiated on first use.
        static TransportFactory instance;
        return instance;
    }

    // Remove copy and assignment constructors.
    TransportFactory(TransportFactory const &) = delete;
    void operator=(TransportFactory const &) = delete;

    /**
     * @brief Create a new transport.
     * @param uri Unified resource identifier of the transport.
     * @return A smart pointer to the newly created transport instance on success, nullptr on error.
     *         Ownership is tracked in the factory; if you want to manually free the resource,
     *         set its value to nullptr so the factory knows about it.
     */
    std::unique_ptr<ITransport> create(const std::string &uri);

    /**
     * @brief Open an existing transport connection.
     * @param uri Unified resource identifier of the transport.
     * @return A pointer to the newly created transport instance on success, nullptr on error.
     *         Ownership is tracked in the factory; if you want to manually free the resource,
     *         set its value to nullptr so the factory knows about it.
     */
    std::unique_ptr<ITransport> open(const std::string &uri);

private:
    TransportFactory() = default;

    std::unique_ptr<ITransport> create_shm(const std::string &uri);
    std::unique_ptr<ITransport> create_tcp(const std::string &uri);
    std::unique_ptr<ITransport> open_shm(const std::string &uri);
    std::unique_ptr<ITransport> open_tcp(const std::string &uri);

    /// internal bookkeeping to cleanup transport instances
    std::vector<ITransport *> m_instances;
};

} // namespace ipc
} // namespace sph

#endif // SPH_IPC_TRANSPORT_FACTORY_H
