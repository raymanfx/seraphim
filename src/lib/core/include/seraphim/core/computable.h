/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_COMPUTABLE_H
#define SPH_CORE_COMPUTABLE_H

namespace sph {
namespace core {

/**
 * @brief Computable interface.
 *
 * Derive from this class to implement an algorithm that computes something.
 * The interface supports computation target switching via \ref set_target.
 * Supported targets are defined in \ref target_t.
 */
class IComputable {
public:
    virtual ~IComputable() = default;

    /**
     * @brief Computation target.
     * These are all the targets which may be supported by the algorithm.
     */
    enum target_t {
        /// CPU computation target
        TARGET_CPU,
        /// GPU computation target (using OpenCL kernels)
        TARGET_OPENCL,
        /// GPU computation target (using OpenCL FP16 kernels)
        TARGET_OPENCL_FP16,
        /// Vision Processing Unit (VPU) target
        TARGET_VPU
    };

    /**
     * @brief Set the computation target.
     * @param target The target (must be one of \ref target_t).
     * @return true if the target is supported by the implementation, false otherwise.
     *         Note that a return value of true does not guarantee that computation will now happen
     *         on the selected target; it is merely a hint to the implementation.
     */
    virtual bool set_target(const target_t &target) = 0;

protected:
    IComputable() = default;
};

} // namespace core
} // namespace sph

#endif // SPH_CORE_COMPUTABLE_H
