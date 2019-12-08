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

#ifdef WITH_OPENCV
#include "seraphim/gui/opencv_window.h"
#endif

using namespace sph;
using namespace sph::gui;

std::unique_ptr<Window> WindowFactory::create(const std::string &title, Impl impl) {
    if (impl == Impl::AUTO) {
        impl = WindowFactory::default_impl();
    }

    switch (impl) {
#ifdef WITH_GLFW
    case Impl::GLFW:
        return std::unique_ptr<Window>(new GLFWWindow(title));
#endif
#ifdef WITH_OPENCV
    case Impl::OPENCV:
        return std::unique_ptr<Window>(new OpenCVWindow(title));
#endif
    default:
        break;
    }

    SPH_THROW(RuntimeException, "No suitable backend for window");
}

WindowFactory::Impl WindowFactory::default_impl() {
#if defined(WITH_GLFW)
    return Impl::GLFW;
#elif defined(WITH_OPENCV)
    return Impl::OPENCV;
#else
    SPH_THROW(RuntimeException, "No window implementation available");
#endif
}
