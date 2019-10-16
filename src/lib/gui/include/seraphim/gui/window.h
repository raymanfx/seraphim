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
class Window {
public:
    virtual ~Window() = default;

    /**
     * @brief Render an image inside the window. The data is immediately uploaded to the GPU.
     *        Throws sph::InvalidArgumentException in case of unsupported format.
     * @param img Input image to be rendered.
     * @return True on success, false otherwise.
     */
    virtual void show(const sph::Image &img) = 0;
};

} // namespace gui
} // namespace sph

#endif // SPH_GUI_WINDOW_H
