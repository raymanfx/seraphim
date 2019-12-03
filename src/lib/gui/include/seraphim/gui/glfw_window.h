/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_GUI_GLFW_WINDOW_H
#define SPH_GUI_GLFW_WINDOW_H

#include <atomic>
#include <mutex>
#include <thread>

#include "glfw.h"
#include "window.h"

namespace sph {
namespace gui {

/**
 * @brief OpenGL based window.
 *
 * Frames are rendered using GLES.
 */
class GLFWWindow : public Window {
public:
    /**
     * @brief GL window implementation.
     *        Throws sph::RuntimeException if GL initialization fails.
     * @param title Window title (for UX).
     */
    explicit GLFWWindow(const std::string &title);
    ~GLFWWindow() override;

    void show(const sph::Image &img) override;

private:
    /// window impl
    GLFWwindow *m_window = nullptr;

    /// ui thread for event emitting etc
    std::thread m_ui_thread;
    std::atomic<bool> m_ui_active;

    /// window width
    int m_width = 0;
    /// window height
    int m_height = 0;

    /// GL shader program
    GLuint m_shader_program = 0;

    /// GL texture
    GLuint m_texture = 0;

    /// GL image properties
    GLint m_input_internal_format = 0;
    GLenum m_input_format = 0;
    GLenum m_input_type = 0;

    /// Vertex Array Object (VAO)
    GLuint m_vao = 0;

    /// Vertex Buffer Object (VBO)
    GLuint m_vbo = 0;

    /// Element Buffer Object (EBO)
    GLuint m_ebo = 0;

    /// initialize GL properties
    /// Throws sph::RuntimeException in case of errors.
    void init_gl();
    /// deinitialize GL
    void terminate_gl();
};

} // namespace gui
} // namespace sph

#endif // SPH_GUI_GLFW_WINDOW_H
