/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_GUI_WINDOW_H
#define SPH_GUI_WINDOW_H

#include <seraphim/core/image.h>
#include <string>

namespace sph {
namespace gui {

/**
 * @brief Window interface.
 *
 * Implement this interface to provide a graphical window element which can render images.
 * Depending on the underlying implementation and the image buffer format, the buffer may be
 * converted before the actual rendering happens.
 */
class IWindow {
public:
    virtual ~IWindow() = default;

    /**
     * @brief Create and initialize the window.
     * @return True on success, false otherwise.
     */
    virtual bool create(const std::string &title) = 0;

    /**
     * @brief Destroy the window and release any associated resources.
     */
    virtual void destroy() = 0;

    /**
     * @brief Render an image inside the window. The data is immediately uploaded to the GPU.
     * @param img Input image to be rendered.
     * @return True on success, false otherwise.
     */
    virtual bool show(const sph::core::Image &img) = 0;
};

} // namespace gui
} // namespace sph

#endif // SPH_GUI_WINDOW_H
