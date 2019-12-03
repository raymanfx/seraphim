/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <thread>

#include "seraphim/except.h"
#include "seraphim/gui/window_factory.h"

#ifdef WITH_GLFW
#include "seraphim/gui/glfw_window.h"
#endif

using namespace sph;
using namespace sph::gui;

std::unique_ptr<Window> WindowFactory::create(const std::string &title, Impl impl) {
    if (impl == Impl::AUTO) {
        // TODO: implement platform detection logic
        impl = Impl::GLFW;
    }

    switch (impl) {
#ifdef WITH_GLFW
    case Impl::GLFW:
        return std::unique_ptr<Window>(new GLFWWindow(title));
#endif
    default:
        break;
    }

    SPH_THROW(RuntimeException, "No suitable backend for window");
}
