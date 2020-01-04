/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_MEMORY_HPP
#define SPH_CORE_MEMORY_HPP

#include <memory>

namespace sph {

/**
 * @brief Convert the object owned by the pointer.
 *
 * Internally, this will create a new smart pointer instance by releasing the old one and taking
 * its owned object, transferring ownership to the newly created pointer.
 *
 * @param ptr The original pointer.
 * @return New pointer holding the requested object type.
 */
template <class T, class U = T>
static inline std::unique_ptr<T> convert_unique(std::unique_ptr<U> &ptr) {
    return std::unique_ptr<T>(static_cast<T *>(ptr.release()));
}

/**
 * @brief Convert the object owned by the pointer.
 *
 * Internally, this will create a new smart pointer instance by releasing the old one and taking
 * its owned object, transferring ownership to the newly created pointer.
 *
 * @param ptr The original pointer.
 * @return New pointer holding the requested object type.
 */
template <class T, class U = T>
static inline std::shared_ptr<T> convert_shared(std::unique_ptr<U> &ptr) {
    return std::shared_ptr<T>(static_cast<T *>(ptr.release()));
}

/**
 * @brief Convert the object owned by the pointer.
 *
 * @param ptr The original pointer.
 * @return New pointer holding the requested object type.
 */
template <class T, class U = T>
static inline std::shared_ptr<T> convert_shared(std::shared_ptr<U> &ptr) {
    return std::static_pointer_cast<T>(ptr);
}

} // namespace sph

#endif // SPH_CORE_MEMORY_HPP
