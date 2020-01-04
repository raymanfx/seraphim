/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_GUI_OPENCV_WINDOW_HPP
#define SPH_GUI_OPENCV_WINDOW_HPP

#include <atomic>
#include <mutex>
#include <thread>

#include "window.hpp"

namespace sph {
namespace gui {

/**
 * @brief OpenCV based window.
 *
 * The actual backend (Qt, GTK+, ...) depends on the OpenCV runtime implementation.
 */
class OpenCVWindow : public Window {
public:
    /**
     * @brief OpenCV window implementation.
     * @param title Window title (for UX).
     */
    explicit OpenCVWindow(const std::string &title);
    ~OpenCVWindow() override;

    void show(const sph::Image &img) override;

private:
    /// Window title.
    std::string m_title;
};

} // namespace gui
} // namespace sph

#endif // SPH_GUI_OPENCV_WINDOW_HPP
