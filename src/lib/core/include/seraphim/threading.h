/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_THREADING_H
#define SPH_CORE_THREADING_H

#include <mutex>

namespace sph {

/**
 * @brief Thin wrapper to synchronize an entire object.
 *
 * Note that this does not synchronize all access to the wrapped instance, but only guards the
 * access of separate threads to the same instance of this class or access from multiple instances
 * of this class to the wrapped instance, regardless of threading.
 */
template <class T> class Synchronized final {
public:
    /**
     * @brief Wrapper instance providing synchronized access.
     *
     * To be able to use this thin wrapper, a class has to implement the @ref Synchronizeable
     * marker interface.
     *
     * @param parent The "real" instance to be synchronized.
     */
    explicit Synchronized(T &parent) : m_ref(parent), m_lock(parent.m_mutex) {}

    /**
     * @brief Default move constructor.
     */
    Synchronized(Synchronized &&) = default;

    /**
     * @brief Pass through class member access to the "real" instance.
     * @return Pointer to the locked class instance.
     */
    T *operator->() noexcept { return &m_ref; }

private:
    Synchronized(const Synchronized &) = delete;
    Synchronized &operator=(const Synchronized &) = delete;
    Synchronized &operator=(Synchronized &&) = delete;

    /// Reference to the parent class to wire up class member access.
    T &m_ref;

    /// Locks the parent classes mutex.
    std::unique_lock<std::mutex> m_lock;
};

/**
 * @brief Marker interface for thread safe instance access.
 *
 * Classes implementing this interface can be accessed in an atomic fashion.
 * This facilitates easy thread safe classes.
 */
template <class T> class Synchronizeable {
public:
    virtual ~Synchronizeable() = default;

    /**
     * @brief Thread safe access to the instance.
     *
     * All actions on the class which go through this thin wrapper are atomic, regardless of which
     * thread they originate from.
     *
     * @return The synchronized instance which is locked internally.
     */
    Synchronized<T> synchronized() { return Synchronized<T>(*(static_cast<T *>(this))); }

    /**
     * @brief Thread safe access to children of the class implementing this marker interface.
     *
     * @see @synchronized
     */
    template <class Derived> Synchronized<Derived> synchronized() {
        return Synchronized<Derived>(*(static_cast<Derived *>(this)));
    }

protected:
    /// Since this is a marker interface, disallow arbitrary instantiation.
    Synchronizeable() = default;

private:
    /// the @ref Synchronized wrapper must have access to the mutex
    template <class T_> friend class Synchronized;

    /// mutex used by the @ref Synchronized wrapper
    std::mutex m_mutex;
};

} // namespace sph

#endif // SPH_CORE_THREADING_H
