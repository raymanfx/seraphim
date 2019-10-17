/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include "seraphim/ipc/transport_factory.h"
#include "seraphim/except.h"
#include "seraphim/ipc/shm_transport.h"
#include "seraphim/ipc/tcp_transport.h"

using namespace sph;
using namespace sph::ipc;

std::unique_ptr<Transport> TransportFactory::create(const std::string &uri) {
    // delegate the real instance creation to helper functions
    if (uri.rfind("shm", 0) == 0) {
        return create_shm(uri);
    } else if (uri.rfind("tcp", 0) == 0) {
        return create_tcp(uri);
    }

    SPH_THROW(InvalidArgumentException, std::string("Cannot handle URI: ") + uri);
}

std::unique_ptr<Transport> TransportFactory::open(const std::string &uri) {
    // delegate the real instance creation to helper functions
    if (uri.rfind("shm", 0) == 0) {
        return open_shm(uri);
    } else if (uri.rfind("tcp", 0) == 0) {
        return open_tcp(uri);
    }

    SPH_THROW(InvalidArgumentException, std::string("Cannot handle URI: ") + uri);
}

std::unique_ptr<Transport> TransportFactory::create_shm(const std::string &uri) {
    std::unique_ptr<SharedMemoryTransport> instance;
    std::string name;
    long size;

    // parse the name and size from the description
    size_t name_start = uri.rfind("/");
    size_t size_start = uri.rfind(":");
    if (name_start == std::string::npos || size_start == std::string::npos ||
        name_start >= size_start) {
        SPH_THROW(InvalidArgumentException, "Missing or malformed delimiters (\"/\" and \":\")");
    }

    // offset positions to capture the actual properties
    name_start++;
    size_start++;

    name = uri.substr(name_start, size_start - name_start - 1);
    if (name.empty()) {
        SPH_THROW(InvalidArgumentException, "Failed to parse memory region name");
    }

    try {
        size = std::stol(uri.substr(size_start));
    } catch (std::invalid_argument) {
        SPH_THROW(InvalidArgumentException, "Failed to parse memory region size");
    }

    // actually create the transport instance
    instance = std::unique_ptr<SharedMemoryTransport>(new SharedMemoryTransport());
    if (!instance->create(name, size)) {
        SPH_THROW(RuntimeException, "Failed to create memory region");
    }

    return std::unique_ptr<Transport>(std::move(instance));
}

std::unique_ptr<Transport> TransportFactory::create_tcp(const std::string &uri) {
    std::unique_ptr<TCPTransport> instance;
    std::string address;
    int port;

    // parse the address and port from the description
    size_t address_start = uri.rfind("/");
    size_t port_start = uri.rfind(":");
    if (address_start == std::string::npos || port_start == std::string::npos ||
        address_start >= port_start) {
        SPH_THROW(InvalidArgumentException, "Missing or malformed delimiters (\"/\" and \":\")");
    }

    // offset positions to capture the actual properties
    address_start++;
    port_start++;

    address = uri.substr(address_start, port_start - address_start - 1);
    try {
        port = std::stoi(uri.substr(port_start));
    } catch (std::invalid_argument) {
        SPH_THROW(InvalidArgumentException, "Failed to parse network port");
    }
    // port must be a uint16_t
    if (port < 0 || port > 65535) {
        SPH_THROW(InvalidArgumentException, "Invalid network port number");
    }

    // validate IPv4/6 address
    net::Socket::Family family;
    struct sockaddr_in sa;
    struct sockaddr_in6 sa6;
    if (inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) == 1) {
        family = net::Socket::Family::INET;
    } else if (inet_pton(AF_INET6, address.c_str(), &(sa6.sin6_addr)) == 1) {
        family = net::Socket::Family::INET6;
    } else {
        SPH_THROW(InvalidArgumentException, "Invalid network address");
    }

    // actually create the transport instance
    instance = std::unique_ptr<TCPTransport>(new TCPTransport(family));

    // bind() will throw on error
    if (!instance->bind(static_cast<uint16_t>(port))) {
        return nullptr;
    }

    return std::unique_ptr<Transport>(std::move(instance));
}

std::unique_ptr<Transport> TransportFactory::open_shm(const std::string &uri) {
    std::unique_ptr<SharedMemoryTransport> instance;
    std::string name;

    // parse the name from the description
    size_t name_start = uri.rfind("/");
    if (name_start == std::string::npos) {
        SPH_THROW(InvalidArgumentException, "Failed to parse memory region name");
    }

    // offset positions to capture the actual properties
    name_start++;

    name = uri.substr(name_start);

    // actually create the transport instance
    instance = std::unique_ptr<SharedMemoryTransport>(new SharedMemoryTransport());
    if (!instance->open(name)) {
        SPH_THROW(RuntimeException, "Failed to open memory region");
    }

    return std::unique_ptr<Transport>(std::move(instance));
}

std::unique_ptr<Transport> TransportFactory::open_tcp(const std::string &uri) {
    std::unique_ptr<TCPTransport> instance;
    std::string address;
    int port;

    // parse the address and port from the description
    size_t address_start = uri.rfind("/");
    size_t port_start = uri.rfind(":");
    if (address_start == std::string::npos || port_start == std::string::npos ||
        address_start >= port_start) {
        SPH_THROW(InvalidArgumentException, "Missing or malformed delimiters (\"/\" and \":\")");
    }

    // offset positions to capture the actual properties
    address_start++;
    port_start++;

    address = uri.substr(address_start, port_start - address_start - 1);
    try {
        port = std::stoi(uri.substr(port_start));
    } catch (std::invalid_argument) {
        SPH_THROW(InvalidArgumentException, "Failed to parse network port");
    }

    // port must be a uint16_t
    if (port < 0 || port > 65535) {
        SPH_THROW(InvalidArgumentException, "Invalid network port number");
    }

    // validate IPv4/6 address
    net::Socket::Family family;
    struct sockaddr_in sa;
    struct sockaddr_in6 sa6;
    if (inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) == 1) {
        family = net::Socket::Family::INET;
    } else if (inet_pton(AF_INET6, address.c_str(), &(sa6.sin6_addr)) == 1) {
        family = net::Socket::Family::INET6;
    } else {
        SPH_THROW(InvalidArgumentException, "Invalid network address");
    }

    // actually create the transport instance
    instance = std::unique_ptr<TCPTransport>(new TCPTransport(family));

    // connect() will throw on error
    if (!instance->connect(address, static_cast<uint16_t>(port))) {
        return nullptr;
    }

    return std::unique_ptr<Transport>(std::move(instance));
}
