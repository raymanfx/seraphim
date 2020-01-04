/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_OBJECT_TRACKER_HPP
#define SPH_OBJECT_TRACKER_HPP

#include <future>
#include <list>

#include <seraphim/image.hpp>
#include <seraphim/polygon.hpp>

namespace sph {
namespace object {

/**
 * @brief Object tracker interface.
 *
 * Derive from this class to implement an object tracker.
 * By default, a tracker is only required to implement single object tracking.
 */
class Tracker {
public:
    virtual ~Tracker() = default;

    /**
     * @brief Initialize the tracker with a known bounding box.
     * @param img Input image.
     * @param rect Rectangle surrounding the object to be tracked.
     */
    virtual void init(const sph::Image &img, const sph::Polygon<int> &rect) = 0;

    /**
     * @brief Track an object in a frame.
     *
     * If no object has been registered via @ref init, an exception will be thrown.
     *
     * @param img Input image.
     * @return Bounding box of the object in the frame. Empty if no object could be tracked.
     */
    virtual sph::Polygon<int> predict(const sph::Image &img) = 0;
};

/**
 * @brief Multi object tracker.
 *
 * Creates one tracker instance per object.
 * Tracker instances are always executed asynchronously to allow for parallelization.
 * At the end of the function, all workers are joined again.
 */
template <class T> class MultiTracker {
public:
    MultiTracker() = default;

    /**
     * @brief Initialize the tracker with known bounding boxes.
     * @param img Input image.
     * @param rects Rectangles surrounding the objects to be tracked.
     */
    void init(const sph::Image &img, const std::vector<sph::Polygon<int>> &rects) {
        std::vector<std::future<void>> handles;

        std::cout << "TRACK 0" << std::endl;
        // create workers if necessary
        m_trackers.resize(rects.size());

        std::cout << "TRACK 1" << std::endl;
        for (size_t i = 0; i < rects.size(); i++) {
#if 0
            // start async workers
            handles.emplace_back(std::async([&]() {
                std::next(m_trackers.begin(), i)->init(img, rects[i]);
            }));
#else
            std::next(m_trackers.begin(), i)->init(img, rects[i]);
#endif
        }

        std::cout << "TRACK 2" << std::endl;
        for (auto &handle : handles) {
            // join the workers
            handle.get();
        }
        std::cout << "TRACK 3" << std::endl;
    }

    /**
     * @brief Track multiple objects in a frame.
     *
     * @param img Input image.
     * @return Bounding boxes of the objects in the frame. Empty if no objects could be tracked.
     */
    std::vector<sph::Polygon<int>> track(const sph::Image &img) {
        std::vector<std::future<void>> handles;
        std::vector<sph::Polygon<int>> rects;

        for (auto &tracker : m_trackers) {
            // start async workers
            handles.emplace_back(std::async([&]() { rects.emplace_back(tracker.track(img)); }));
        }

        for (auto &handle : handles) {
            // join the workers
            handle.get();
        }

        return rects;
    }

private:
    /// One tracker instance per object
    std::list<T> m_trackers;
};

} // namespace object
} // namespace sph

#endif // SPH_OBJECT_TRACKER_HPP
