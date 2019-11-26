/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_GUI_GLFW_H
#define SPH_GUI_GLFW_H

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <iostream>

#include "glad/glad.h"
#include "seraphim/except.h"

namespace sph {
namespace gui {

// glfw must only be initialized and destroyed once per process
static bool glfw_initialized = false;
static int glfw_windows = 0;

static void glfw_error_callback(int, const char *msg) {
    std::cout << "[ERROR] GLFW: " << msg << std::endl;
}

/**
 * @brief Initialize GLFW.
 *
 * This only ever runs once during the lifetime of the process.
 *
 * An error callback is registered to print failures.
 * Throws sph::RuntimeException in case of errors.
 */
inline void glfw_init() {
    if (!glfw_initialized) {
        glfwSetErrorCallback(glfw_error_callback);
        glfw_initialized = glfwInit() == GLFW_TRUE;
        if (!glfw_initialized) {
            SPH_THROW(sph::RuntimeException, "Failed to initialize GLFW");
        }
    }
}

/**
 * @brief Terminate GLFW.
 *
 * This only ever runs once during the lifetime of the process.
 * After this has been called, all GLFW ops will fail until glfw_init() is called again.
 */
inline void glfw_terminate() {
    if (glfw_initialized && glfw_windows == 0) {
        glfwTerminate();
        glfw_initialized = false;
    }
}

/**
 * @brief Create a GLFW window.
 *        Just a wrapper around glfwCreateWindow() that keeps a reference count on the created
 *        windows.
 * @param width Width in pixels.
 * @param height Height in pixels.
 * @param title Window title used by the window toolkit.
 * @return The created window instance on success or nullptr on error.
 */
inline GLFWwindow *glfw_create_window(int width, int height, const char *title) {
    GLFWwindow *window;

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (window) {
        glfw_windows++;
    }
    return window;
}

/**
 * @brief Destroy a GLFW window.
 * @param window Pointer to the instance as returned by glfw_create_window().
 */
inline void glfw_destroy_window(GLFWwindow *window) {
    glfwDestroyWindow(window);
    if (glfw_windows > 0) {
        glfw_windows--;
    }
}

} // namespace gui
} // namespace sph

#endif // SPH_GUI_GLFW_H
