/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_GUI_WINDOW_HPP
#define SPH_GUI_WINDOW_HPP

#include <functional>
#include <future>

#include <seraphim/image.hpp>
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

    enum Event { CLOSE = 0x1 };

    /**
     * @brief Publish an event to subscribers.
     * @param event Event type.
     */
    void publish(Event event) {
        std::vector<std::future<void>> handles;

        for (const auto &sub : m_subscribers) {
            if (sub.first & event) {
                handles.push_back(
                    std::async(std::launch::async, [sub, event]() { sub.second(event); }));
            }
        }

        // all futures are joined here (when they go out of scope)
    }

    /**
     * @brief Event handling function.
     */
    typedef std::function<void(Event event)> EventHandler;

    /**
     * @brief Subscribe to window events.
     * @param handler The event handler to be called when events happen.
     * @param mask Bitmask of events the subscriber is interested in.
     */
    void subscribe(EventHandler handler, Event mask) {
        m_subscribers.emplace_back(std::make_pair(mask, handler));
    }

    /**
     * @brief Render an image inside the window. The data is immediately uploaded to the GPU.
     *
     * Throws sph::InvalidArgumentException in case of unsupported format.
     *
     * @param img Input image to be rendered.
     * @return True on success, false otherwise.
     */
    virtual void show(const sph::Image &img) = 0;

protected:
    std::vector<std::pair<Event, EventHandler>> m_subscribers;
};

} // namespace gui
} // namespace sph

#endif // SPH_GUI_WINDOW_HPP
