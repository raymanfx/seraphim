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

class IService {
public:
    virtual ~IService() = default;

    virtual bool handle_request(const Seraphim::Request &req, Seraphim::Response &res) = 0;

protected:
    // disallow default construction, copy construction
    IService() = default;
    IService(const IService &) = default;
};

} // namespace backend
} // namespace sph

#endif // SPH_ISERVICE_H
