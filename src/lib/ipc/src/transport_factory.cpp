/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include "seraphim/ipc/transport_factory.h"
#include "seraphim/ipc/shm_transport.h"
#include "seraphim/ipc/tcp_transport.h"

using namespace sph::ipc;

std::unique_ptr<ITransport> TransportFactory::create(const std::string &uri) {
    std::unique_ptr<ITransport> instance = nullptr;

    // delegate the real instance creation to helper functions
    if (uri.rfind("shm", 0) == 0) {
        instance = create_shm(uri);
    } else if (uri.rfind("tcp", 0) == 0) {
        instance = create_tcp(uri);
    }

    return instance;
}

std::unique_ptr<ITransport> TransportFactory::open(const std::string &uri) {
    std::unique_ptr<ITransport> instance = nullptr;

    // delegate the real instance creation to helper functions
    if (uri.rfind("shm", 0) == 0) {
        instance = open_shm(uri);
    } else if (uri.rfind("tcp", 0) == 0) {
        instance = open_tcp(uri);
    }

    return instance;
}

std::unique_ptr<ITransport> TransportFactory::create_shm(const std::string &uri) {
    SharedMemoryTransport *instance;
    std::string name;
    long size;

    // parse the name and size from the description
    size_t name_start = uri.rfind("/");
    size_t size_start = uri.rfind(":");
    if (name_start == std::string::npos || size_start == std::string::npos ||
        name_start >= size_start) {
        return nullptr;
    }
    // offset positions to capture the actual properties
    name_start++;
    size_start++;

    name = uri.substr(name_start, size_start - name_start - 1);
    if (name.empty()) {
        return nullptr;
    }
    try {
        size = std::stol(uri.substr(size_start));
    } catch (std::invalid_argument) {
        return nullptr;
    }

    // actually create the transport instance
    instance = new SharedMemoryTransport();
    if (!instance->create(name, size)) {
        delete instance;
        return nullptr;
    }

    return std::unique_ptr<ITransport>(instance);
}

std::unique_ptr<ITransport> TransportFactory::create_tcp(const std::string &uri) {
    TCPTransport *instance;
    std::string address;
    int port;

    // parse the address and port from the description
    size_t address_start = uri.rfind("/");
    size_t port_start = uri.rfind(":");
    if (address_start == std::string::npos || port_start == std::string::npos ||
        address_start >= port_start) {
        return nullptr;
    }
    // offset positions to capture the actual properties
    address_start++;
    port_start++;

    address = uri.substr(address_start, port_start - address_start - 1);
    try {
        port = std::stoi(uri.substr(port_start));
    } catch (std::invalid_argument) {
        return nullptr;
    }
    // port must be a uint16_t
    if (port < 0 || port > 65535) {
        return nullptr;
    }

    // validate IPv4/6 address
    int domain = -1;
    struct sockaddr_in sa;
    struct sockaddr_in6 sa6;
    if (inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) == 1) {
        domain = AF_INET;
    } else if (inet_pton(AF_INET6, address.c_str(), &(sa6.sin6_addr)) == 1) {
        domain = AF_INET6;
    }
    if (domain == -1) {
        return nullptr;
    }

    // actually create the transport instance
    instance = new TCPTransport(domain);
    if (!instance->bind(static_cast<uint16_t>(port))) {
        delete instance;
        return nullptr;
    }

    return std::unique_ptr<ITransport>(instance);
}

std::unique_ptr<ITransport> TransportFactory::open_shm(const std::string &uri) {
    SharedMemoryTransport *instance;
    std::string name;

    // parse the name from the description
    size_t name_start = uri.rfind("/");
    if (name_start == std::string::npos) {
        return nullptr;
    }
    // offset positions to capture the actual properties
    name_start++;

    name = uri.substr(name_start);

    // actually create the transport instance
    instance = new SharedMemoryTransport();
    if (!instance->open(name)) {
        delete instance;
        return nullptr;
    }

    return std::unique_ptr<ITransport>(instance);
}

std::unique_ptr<ITransport> TransportFactory::open_tcp(const std::string &uri) {
    TCPTransport *instance;
    std::string address;
    int port;

    // parse the address and port from the description
    size_t address_start = uri.rfind("/");
    size_t port_start = uri.rfind(":");
    if (address_start == std::string::npos || port_start == std::string::npos ||
        address_start >= port_start) {
        return nullptr;
    }
    // offset positions to capture the actual properties
    address_start++;
    port_start++;

    address = uri.substr(address_start, port_start - address_start - 1);
    try {
        port = std::stoi(uri.substr(port_start));
    } catch (std::invalid_argument) {
        return nullptr;
    }
    // port must be a uint16_t
    if (port < 0 || port > 65535) {
        return nullptr;
    }

    // validate IPv4/6 address
    int domain = -1;
    struct sockaddr_in sa;
    struct sockaddr_in6 sa6;
    if (inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) == 1) {
        domain = AF_INET;
    } else if (inet_pton(AF_INET6, address.c_str(), &(sa6.sin6_addr)) == 1) {
        domain = AF_INET6;
    }
    if (domain == -1) {
        return nullptr;
    }

    // actually create the transport instance
    instance = new TCPTransport(domain);
    if (!instance->connect(address, static_cast<uint16_t>(port))) {
        delete instance;
        return nullptr;
    }

    return std::unique_ptr<ITransport>(instance);
}
