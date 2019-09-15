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
#include <glad/glad.h>
#include <iostream>

namespace sph {
namespace gui {

// glfw must only be initialized and destroyed once per process
static bool glfw_initialized = false;
static int glfw_windows = 0;

static void glfw_error_callback(int, const char *msg) {
    std::cout << "[ERROR] GLFW: " << msg << std::endl;
}

static bool glfw_init() {
    if (glfw_initialized) {
        return true;
    }
    glfwSetErrorCallback(glfw_error_callback);
    glfw_initialized = glfwInit() == GLFW_TRUE;
    return glfw_initialized;
}

static void glfw_terminate() {
    if (!glfw_initialized) {
        return;
    }
    if (glfw_windows > 0) {
        return;
    }
    glfwTerminate();
}

static GLFWwindow *glfw_create_window(const int &width, const int &height, const char *title,
                                      GLFWmonitor *monitor, GLFWwindow *share) {
    GLFWwindow *window;

    // TODO: GLEW MX supports multiple contexts (thus multiple windows), but is not available in
    // most distributions in the binary glew package
    if (glfw_windows > 0) {
        return nullptr;
    }

    window = glfwCreateWindow(width, height, title, monitor, share);
    if (window) {
        glfw_windows++;
    }
    return window;
}

static void glfw_destroy_window(GLFWwindow *window) {
    if (window) {
        glfwDestroyWindow(window);
        if (glfw_windows > 0) {
            glfw_windows--;
        }
    }
}

} // namespace gui
} // namespace sph

#endif // SPH_GUI_GLFW_H
