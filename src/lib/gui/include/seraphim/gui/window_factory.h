/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_GUI_WINDOW_FACTORY_H
#define SPH_GUI_WINDOW_FACTORY_H

#include <atomic>
#include <mutex>
#include <thread>

#include "window.h"

namespace sph {
namespace gui {

/**
 * @brief Factory to create windows.
 *
 * Windows can be created using a multitude of backend implementations.
 * Note that backend support might depend on the platform.
 */
class WindowFactory {
public:
    /**
     * @brief Singleton class instance.
     * @return The single, static instance of this class.
     */
    static WindowFactory &Instance() {
        // Guaranteed to be destroyed, instantiated on first use.
        static WindowFactory instance;
        return instance;
    }

    // Remove copy and assignment constructors.
    WindowFactory(WindowFactory const &) = delete;
    void operator=(WindowFactory const &) = delete;

    enum class Impl { AUTO, GLFW, OPENCV };

    /**
     * @brief GL window implementation.
     *
     * Throws sph::RuntimeException if no suitable backend is available.
     *
     * @param title Window title (for UX).
     */
    static std::unique_ptr<Window> create(const std::string &title, Impl impl = Impl::AUTO);

    /**
     * @brief Default window impl for the platform.
     * @return Window impl enum.
     */
    static Impl default_impl();

private:
    WindowFactory() = default;
};

} // namespace gui
} // namespace sph

#endif // SPH_GUI_WINDOW_FACTORY_H
