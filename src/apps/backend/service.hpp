/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_SERVICE_HPP
#define SPH_SERVICE_HPP

#include <Seraphim.pb.h>

namespace sph {
namespace backend {

class Service {
public:
    virtual ~Service() = default;

    /**
     * @brief Handle a request message.
     * @param req The request.
     * @param res The response, filled by this method.
     * @return True on success, false otherwise (e.g. if this service does not handle this kind of
     * request).
     */
    virtual bool handle_request(const Seraphim::Request &req, Seraphim::Response &res) = 0;

protected:
    Service() = default;
    // disallow copy and move construction
    Service(const Service &) = delete;
    Service(Service &&) = delete;
    // disallow copy and move assignment
    Service &operator=(const Service &) = delete;
    Service &operator=(Service &&) = delete;
};

} // namespace backend
} // namespace sph

#endif // SPH_SERVICE_HPP
