/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_GUI_GL_WINDOW_H
#define SPH_GUI_GL_WINDOW_H

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
class GLWindow : public IWindow {
public:
    GLWindow();
    ~GLWindow() override;

    bool create(const std::string &title) override;
    void destroy() override;
    bool show(const sph::core::Image &img) override;

private:
    /// window impl
    GLFWwindow *m_window;

    /// ui thread for event emitting etc
    std::thread m_ui_thread;
    std::atomic<bool> m_ui_active;

    /// window width
    int m_width;
    /// window height
    int m_height;

    /// GL shader program
    GLuint m_shader_program;

    /// GL texture
    GLuint m_texture;

    /// GL image properties
    GLint m_input_internal_format;
    GLenum m_input_format;
    GLenum m_input_type;

    /// Vertex Array Object (VAO)
    GLuint m_vao;

    /// Vertex Buffer Object (VBO)
    GLuint m_vbo;

    /// Element Buffer Object (EBO)
    GLuint m_ebo;

    /// initialize GL properties
    bool init_gl();
    /// deinitialize GL
    void terminate_gl();
};

} // namespace gui
} // namespace sph

#endif // SPH_GUI_GL_WINDOW_H
