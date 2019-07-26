/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_SERVICE_H
#define SPH_SERVICE_H

#include <Seraphim.pb.h>

namespace sph {
namespace backend {

class Service {
public:
    virtual ~Service() = default;

    virtual bool handle_request(const Seraphim::Request &req, Seraphim::Response &res) = 0;

protected:
    Service() = default;
};

} // namespace backend
} // namespace sph

#endif // SPH_SERVICE_H