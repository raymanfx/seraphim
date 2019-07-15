/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_ISERVER_H
#define SPH_ISERVER_H

#include <Seraphim.pb.h>
#include <functional>

namespace sph {
namespace backend {

class IServer {
public:
    virtual ~IServer() = default;

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
    // disallow default construction, copy construction
    IServer() = default;
    IServer(const IServer &) = default;
};

} // namespace backend
} // namespace sph

#endif // SPH_ISERVER_H
