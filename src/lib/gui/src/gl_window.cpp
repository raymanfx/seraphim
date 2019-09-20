/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <thread>

#include "seraphim/gui/gl_window.h"

using namespace sph::gui;

GLWindow::~GLWindow() {
    destroy();
}

bool GLWindow::create(const std::string &title) {
    if (!glfw_init()) {
        return false;
    }

    m_window = glfw_create_window(1, 1, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        return false;
    }
    m_width = 1;
    m_height = 1;

    glfwMakeContextCurrent(m_window);
    if (!gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        return false;
    }

    // wait one frame until swapping the buffers (vsync)
    glfwSwapInterval(1);

    if (!init_gl()) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        return false;
    }

    m_ui_active = true;
    m_ui_thread = std::thread([&]() {
        while (m_ui_active) {
            // TODO: check glfwWindowShouldClose(m_window) and act accordingly
            glfwPollEvents();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    return true;
}

void GLWindow::destroy() {
    if (m_window) {
        m_ui_active = false;
        if (m_ui_thread.joinable()) {
            m_ui_thread.join();
        }
        terminate_gl();
        glfw_destroy_window(m_window);
        m_window = nullptr;
    }

    glfw_terminate();
}

bool GLWindow::show(const sph::core::Image &img) {
    static bool setup_texture = true;
    int w = static_cast<int>(img.width());
    int h = static_cast<int>(img.height());
    GLint input_internal_format;
    GLenum input_format;
    GLenum input_type;

    if (!m_window) {
        return false;
    }

    // the current window context must be made current because OpenGL is a state machine
    glfwMakeContextCurrent(m_window);

    // use fast 4-byte alignment (default anyway) if possible
    // see: https://stackoverflow.com/a/16812529
    glPixelStorei(GL_UNPACK_ALIGNMENT, static_cast<GLint>(img.stride() & 3 ? 1 : 4));

    // set amount of pixels of one complete row in data
    // see: https://stackoverflow.com/a/16812529
    glPixelStorei(GL_UNPACK_ROW_LENGTH, static_cast<GLint>(img.width()));

    switch (img.pixfmt()) {
    case sph::core::Pixelformat::Enum::RGB24:
    case sph::core::Pixelformat::Enum::RGB32:
        input_internal_format = GL_RGB;
        input_format = GL_RGB;
        input_type = GL_UNSIGNED_BYTE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
        break;
    case sph::core::Pixelformat::Enum::BGR24:
    case sph::core::Pixelformat::Enum::BGR32:
        input_internal_format = GL_RGB;
        input_format = GL_RGB;
        input_type = GL_UNSIGNED_BYTE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
        break;
    case sph::core::Pixelformat::Enum::GRAY8:
        input_internal_format = GL_R8;
        input_format = GL_RED;
        input_type = GL_UNSIGNED_BYTE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
        break;
    default:
        return false;
    }

    // recreate the texture if width or height have changed
    if (w != m_width || h != m_height) {
        m_width = w;
        m_height = h;
        glfwSetWindowSize(m_window, w, h);
        setup_texture = true;

        // enforce original aspect ratio
        glfwSetWindowAspectRatio(m_window, w, h);
    }

    // recreate the texture if internal parameters have changed
    if (input_internal_format != m_input_internal_format || input_format != m_input_format ||
        input_type != m_input_type) {
        m_input_internal_format = input_internal_format;
        m_input_format = input_format;
        m_input_type = input_type;
        setup_texture = true;
    }

    if (setup_texture) {
        // Create the texture
        glTexImage2D(GL_TEXTURE_2D, // Type of texture
                     0,             // Pyramid level (for mip-mapping) - 0 is the top level
                     m_input_internal_format, // Internal colour format to convert to
                     m_width,                 // Image width
                     m_height,                // Image height
                     0,                       // Border width in pixels (can either be 1 or 0)
                     m_input_format, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                     m_input_type,   // Image data type
                     nullptr);       // Don't copy data, just create the texture buffer
        setup_texture = false;
    }

    // Update the texture
    glTexSubImage2D(GL_TEXTURE_2D,  // Type of texture
                    0,              // Pyramid level (for mip-mapping) - 0 is the top level
                    0,              // x offset
                    0,              // y offset
                    m_width,        // Image width
                    m_height,       // Image height
                    m_input_format, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                    m_input_type,   // Image data type
                    img.data());    // The actual image data itself

    // setup the viewport
    glfwGetFramebufferSize(m_window, &w, &h);
    glViewport(0, 0, w, h);

    // bind the texture
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    // draw vertices
    glUseProgram(m_shader_program);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glfwSwapBuffers(m_window);

    return true;
}

bool GLWindow::init_gl() {
    const char *vertex_shader_source =
        "#version 300 es\n"
        // input vertex data, different for all executions of this shader
        "layout (location = 0) in vec3 vertexPosition_modelspace;\n"
        "layout (location = 1) in vec2 vertexUV;\n"
        "\n"
        // output data, will be interpolated for each fragment
        "out vec2 UV;\n"
        "\n"
        "void main()\n"
        "{\n"
        // output position of the vertex, in clip space
        "    gl_Position = vec4(vertexPosition_modelspace, 1.0);\n"
        // UV of the vertex, no special space for this one
        "    UV = vertexUV;\n"
        "}\n\0";

    const char *fragment_shader_source = "#version 300 es\n"
                                         "precision mediump float;\n"
                                         // interpolated values from the vertex shaders
                                         "in vec2 UV;\n"
                                         "\n"
                                         // output data
                                         "out vec4 color;\n"
                                         "\n"
                                         // values that stay constant for the whole mesh
                                         "uniform sampler2D textureSampler;\n"
                                         "\n"
                                         "void main()\n"
                                         "{\n"
                                         // output color = color of the texture at the specified UV
                                         "    color = texture(textureSampler, UV);\n"
                                         "}\n\0";

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED" << infoLog << std::endl;
        return false;
    }
    // fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED" << infoLog << std::endl;
        return false;
    }
    // link shaders
    m_shader_program = glCreateProgram();
    glAttachShader(m_shader_program, vertexShader);
    glAttachShader(m_shader_program, fragmentShader);
    glLinkProgram(m_shader_program);
    // check for linking errors
    glGetProgramiv(m_shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_shader_program, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED" << infoLog << std::endl;
        return false;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // Y coordinates are swapped, so we can draw the image from the UV coordinates in a 'normal'
    // fashion this is because OpenGL stores images top-to-bottom (first row starts at -1, -1) by
    // default
    // --------------------------------------------------------------------------------------------
    float vertices[] = {
        // positions          // texture coords
        -1.0f, 1.0f,  0.0f, 0.0f, 0.0f, // bottom left
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top left
        1.0f,  -1.0f, 0.0f, 1.0f, 1.0f, // top right
        1.0f,  1.0f,  0.0f, 1.0f, 0.0f  // bottom right
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure
    // vertex attributes(s).
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          reinterpret_cast<GLvoid *>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // generate OpenGL texture
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenTextures(1, &m_texture);

    // bind to texture handle
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    // set texture interpolation methods for minification and magnification
    // GL_NEAREST is the fastest but also the most basic one
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // set texture clamping method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return true;
}

void GLWindow::terminate_gl() {
    glDeleteTextures(1, &m_texture);
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}
