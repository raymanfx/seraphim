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

    virtual bool run() = 0;
    virtual void terminate() = 0;

    typedef std::function<bool(Seraphim::Message &)> handler_t;

    void on_message(handler_t handler) { m_handler = handler; }

protected:
    Server() = default;
    handler_t m_handler;
};

} // namespace backend
} // namespace sph

#endif // SPH_SERVER_H
