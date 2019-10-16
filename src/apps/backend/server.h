/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_SERVER_H
#define SPH_SERVER_H

#include <Seraphim.pb.h>
#include <functional>

namespace sph {
namespace backend {

class Server {
public:
    virtual ~Server() = default;

    virtual bool init(const std::string &uri) = 0;
    virtual bool run() = 0;
    virtual void terminate() = 0;

    typedef std::function<bool(void *)> event_handler_t;

    enum event_t {
        EVENT_CLIENT_CONNECTED = 0x1,
        EVENT_CLIENT_DISCONNECTED = 0x2,
        EVENT_MESSAGE_INCOMING = 0x4,
        EVENT_MESSAGE_OUTGOING = 0x8
    };

    /**
     * @brief Register an event handler.
     * @param mask The events to listen on (must be one of \ref event_t).
     * @param handler The event handler function.
     */
    virtual void register_event_handler(const event_t &mask, const event_handler_t handler) = 0;

protected:
    Server() = default;
    // disallow copy and move construction
    Server(const Server &) = delete;
    Server(Server &&) = delete;
    // disallow copy and move assignment
    Server &operator=(const Server &) = delete;
    Server &operator=(Server &&) = delete;
};

} // namespace backend
} // namespace sph

#endif // SPH_SERVER_H
