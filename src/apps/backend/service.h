/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_ISERVICE_H
#define SPH_ISERVICE_H

#include <Seraphim.pb.h>

namespace sph {
namespace backend {

class Service {
public:
    virtual ~Service() = default;

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

#endif // SPH_ISERVICE_H
