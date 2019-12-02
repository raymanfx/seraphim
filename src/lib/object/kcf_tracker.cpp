/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <seraphim/except.h>
#include <seraphim/iop/opencv/mat.h>

#include "seraphim/object/kcf_tracker.h"

using namespace sph;
using namespace sph::object;

KCFTracker::KCFTracker() {
    // initialize one tracker instance upfront
    m_tracker_swap = std::async(std::launch::deferred, &KCFTracker::allocate_tracker, this);
}

void KCFTracker::allocate_tracker() {
    size_t next_index = (m_tracker_index + 1) % 2;

    m_tracker[next_index] = cv::TrackerKCF::create();
}

void KCFTracker::init(const sph::Image &img, const sph::Polygon<int> &rect) {
    cv::Mat _img;
    cv::Rect2d _rect;

    _img = iop::cv::from_image(img);
    if (_img.empty()) {
        SPH_THROW(RuntimeException, "Failed to convert image");
    }

    _rect.x = rect.tl().x;
    _rect.y = rect.tl().y;
    _rect.width = rect.width();
    _rect.height = rect.height();

    // clamp to frame boundaries
    _rect.x = _rect.x < 0 ? 0 : _rect.x;
    _rect.y = _rect.y < 0 ? 0 : _rect.y;
    if (_rect.x + _rect.width >= _img.cols) {
        _rect.width = _img.cols - _rect.x - 1;
    }
    if (_rect.y + _rect.height >= _img.rows) {
        _rect.height = _img.rows - _rect.y - 1;
    }

    // ensure a new tracker instance is available
    m_tracker_swap.get();

    // the new instance is ready now, update the index
    m_tracker_index = (m_tracker_index + 1) % 2;

    if (!m_tracker[m_tracker_index]->init(_img, _rect)) {
        SPH_THROW(RuntimeException, "Failed to initialize tracker");
    }

    // OpenCV does not allow tracker reinitialization (even after an object is lost), so we have to
    // forcefully destroy and recreate the object
    m_tracker_swap = std::async(std::launch::async, &KCFTracker::allocate_tracker, this);
}

sph::Polygon<int> KCFTracker::predict(const sph::Image &img) {
    cv::Mat _img;
    cv::Rect2d _rect;
    sph::Polygon<int> rect;

    _img = iop::cv::from_image(img);
    if (_img.empty()) {
        SPH_THROW(RuntimeException, "Failed to convert image");
    }

    if (m_tracker.empty()) {
        SPH_THROW(RuntimeException, "Tracker not initialized");
    }

    m_tracker[m_tracker_index]->update(_img, _rect);
    rect.add_point(Point2i(_rect.tl().x, _rect.tl().y));
    rect.add_point(Point2i(_rect.tl().x + _rect.width, _rect.tl().y));
    rect.add_point(Point2i(_rect.br().x, _rect.br().y));
    rect.add_point(Point2i(_rect.br().x - _rect.width, _rect.br().y));

    return rect;
}
