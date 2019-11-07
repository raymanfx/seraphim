/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "seraphim/ipc/net/udp_socket.h"

using namespace sph;
using namespace sph::ipc::net;

UDPSocket::UDPSocket(Family family)
    : Socket(family, Socket::Type::DATAGRAM, Socket::Protocol::UDP) {
    // allow address reuse by default
    int opt_val = 1;
    set_opt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt_val, sizeof(opt_val));
}
