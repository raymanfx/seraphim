/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_OBJECT_TRACKER_KCF_H
#define SPH_OBJECT_TRACKER_KCF_H

#include <array>
#include <atomic>
#include <future>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <seraphim/computable.h>
#include <vector>

#include "tracker.h"

namespace sph {
namespace object {

/**
 * @brief Kernel correlation features (KCF) tracker.
 *
 * Tracks single objects in frames based on a ground truth (e.g. bounding boxes from an object
 * detector).
 * Thread safety is not provided.
 */
class KCFTracker : public Tracker {
public:
    KCFTracker();

    void init(const sph::Image &img, const sph::Polygon<int> &rect) override;

    sph::Polygon<int> predict(const sph::Image &img) override;

private:
    /// The actual tracker implementation.
    /// Double buffering is used to ensure there is always one instance available for
    /// initialization.
    std::array<cv::Ptr<cv::TrackerKCF>, 2> m_tracker;

    /// Index of the currently active tracker.
    std::atomic<size_t> m_tracker_index = 0;

    /// Must be fulfilled when a new tracker instance is expected.
    std::future<void> m_tracker_swap;

    /// Create a new tracker and add it to the array.
    void allocate_tracker();
};

} // namespace object
} // namespace sph

#endif // SPH_OBJECT_TRACKER_KCF_H
