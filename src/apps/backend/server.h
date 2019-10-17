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
#include <list>

#include "service.h"

namespace sph {
namespace backend {

class Server {
public:
    virtual ~Server() = default;

    virtual bool init(const std::string &uri) = 0;
    virtual bool run() = 0;
    virtual void terminate() = 0;

    typedef std::function<void(void *)> event_handler_t;

    enum EventType {
        EVENT_CLIENT_CONNECTED = 1 << 0,
        EVENT_CLIENT_DISCONNECTED = 1 << 1,
        EVENT_MESSAGE_INBOUND = 1 << 2,
        EVENT_MESSAGE_OUTBOUND = 1 << 3,
        EVENT_MESSAGE_HANDLED = 1 << 4
    };

    inline friend EventType operator|(EventType lhs, EventType rhs) {
        return static_cast<EventType>(static_cast<int>(lhs) | static_cast<int>(rhs));
    }

    /**
     * @brief Register an event handler.
     *
     * In case of multiple handlers being registered for one event, all of them will be notified.
     * @param mask The events to listen on (must be one of \ref event_t).
     * @param handler The event handler function.
     */
    void register_event_handler(EventType mask, const event_handler_t handler) {
        m_event_handlers.emplace_front(std::make_pair(mask, handler));
    }

    /**
     * @brief Register a service.
     *
     * A service handles incoming requests and emits appropriate responses.
     * The service that got registered last will take precedence, effectively overriding any
     * services handling the same kind of requests.
     * @param service The service that handles incoming requests.
     */
    void register_service(std::shared_ptr<Service> service) { m_services.emplace_front(service); }

protected:
    Server() = default;
    // disallow copy and move construction
    Server(const Server &) = delete;
    Server(Server &&) = delete;
    // disallow copy and move assignment
    Server &operator=(const Server &) = delete;
    Server &operator=(Server &&) = delete;

    /**
     * @brief Emit an event to registered listeners.
     * @param event The event that occured, see \ref event_t.
     * @param data An opaque handle to data associated with the event.
     */
    void emit_event(EventType event, void *data) {
        for (const auto &handler : m_event_handlers) {
            if (handler.first & event) {
                handler.second(data);
            }
        }
    }

    /**
     * @brief Relay a request message to registered services.
     * @param msg The message that was received by the server.
     */
    void handle_message(Seraphim::Message &msg) {
        bool handled = false;
        Seraphim::Response res;

        if (!msg.has_req()) {
            // not a request
            msg.mutable_res()->set_status(-1);
            return;
        }

        for (const auto &service : m_services) {
            handled = service->handle_request(msg.req(), res);
            if (handled) {
                emit_event(EVENT_MESSAGE_HANDLED, &msg);
                break;
            }
        }

        msg.mutable_res()->set_status(handled ? 0 : -1);
        msg.mutable_res()->Swap(&res);
    }

    /// Event handlers.
    std::list<std::pair<int, event_handler_t>> m_event_handlers;

    /// Services acting as message handlers.
    std::list<std::shared_ptr<Service>> m_services;
};

} // namespace backend
} // namespace sph

#endif // SPH_SERVER_H
