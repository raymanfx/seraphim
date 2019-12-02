/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_POLYGON_H
#define SPH_CORE_POLYGON_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "point.h"

namespace sph {

/**
 * @brief Polygon object in 2D (euclidian) space.
 * Seraphim uses a top-down coordinate system, meaning the coordinate space origin is in the top
 * left hand corner (0, 0) and the bottom right hand corner is (x_max, y_max).
 * Thus, bl().y > tl().y for example.
 */
template <typename T> class Polygon {
public:
    /**
     * @brief Default constructor for an empty polygon.
     * Points have to be added manually via @ref add_point.
     */
    Polygon() = default;

    /**
     * @brief Create a new polygon shape.
     * @param points List of points defining the polygon.
     */
    template <typename... Points> Polygon(Points &&... points) : m_points{ points... } {}

    /**
     * @brief Check whether the polygon is empty (i.e. undefined).
     * @return True if there are less than three points, false otherwise.
     */
    bool empty() const { return m_points.size() < 3; }

    /**
     * @brief Clear the polygon, removing all its points.
     */
    void clear() { m_points.clear(); }

    /**
     * @brief Get the points that make the polygon shape.
     * @return Set of points in order of appearance.
     */
    std::vector<Point2<T>> points() const { return m_points; }

    /**
     * @brief Add a point to the polygon.
     * @param p Point with type T.
     */
    void add_point(const Point2<T> &p) { m_points.push_back(p); }

    /**
     * @brief operator ==
     * @param rhs Right hand side.
     * @return True if equal, false otherwise.
     */
    bool operator==(const Polygon &rhs) const { return m_points == rhs.m_points; }

    /**
     * @brief Bottom left point.
     * @return Point as set of coordinates with type T.
     */
    Point2<T> bl() const {
        if (empty()) {
            return Point2<T>(0, 0);
        }

        Point2<T> p = m_points[0];
        Polygon rect = bounding_rect();
        Point2<T> rect_bl = rect.points()[0];

        for (const auto &p_ : m_points) {
            // metric: absolute distance to bounding rect
            T cur_dist = std::abs(p.x - rect_bl.x) + std::abs(p.y - rect_bl.y);
            T new_dist = std::abs(p_.x - rect_bl.x) + std::abs(p_.y - rect_bl.y);
            if (new_dist < cur_dist) {
                p = p_;
            }
        }

        return p;
    }

    /**
     * @brief Top left point.
     * @return Point as set of coordinates with type T.
     */
    Point2<T> tl() const {
        if (empty()) {
            return Point2<T>(0, 0);
        }

        Point2<T> p = m_points[0];
        Polygon rect = bounding_rect();
        Point2<T> rect_tl = rect.points()[1];

        for (const auto &p_ : m_points) {
            // metric: absolute distance to bounding rect
            T cur_dist = std::abs(p.x - rect_tl.x) + std::abs(p.y - rect_tl.y);
            T new_dist = std::abs(p_.x - rect_tl.x) + std::abs(p_.y - rect_tl.y);
            if (new_dist < cur_dist) {
                p = p_;
            }
        }

        return p;
    }

    /**
     * @brief Top right point.
     * @return Point as set of coordinates with type T.
     */
    Point2<T> tr() const {
        if (empty()) {
            return Point2<T>(0, 0);
        }

        Point2<T> p = m_points[0];
        Polygon rect = bounding_rect();
        Point2<T> rect_tr = rect.points()[2];

        for (const auto &p_ : m_points) {
            // metric: absolute distance to bounding rect
            T cur_dist = std::abs(p.x - rect_tr.x) + std::abs(p.y - rect_tr.y);
            T new_dist = std::abs(p_.x - rect_tr.x) + std::abs(p_.y - rect_tr.y);
            if (new_dist < cur_dist) {
                p = p_;
            }
        }

        return p;
    }

    /**
     * @brief Bottom right point.
     * @return Point as set of coordinates with type T.
     */
    Point2<T> br() const {
        if (empty()) {
            return Point2<T>(0, 0);
        }

        Point2<T> p = m_points[0];
        Polygon rect = bounding_rect();
        Point2<T> rect_br = rect.points()[3];

        for (const auto &p_ : m_points) {
            // metric: absolute distance to bounding rect
            T cur_dist = std::abs(p.x - rect_br.x) + std::abs(p.y - rect_br.y);
            T new_dist = std::abs(p_.x - rect_br.x) + std::abs(p_.y - rect_br.y);
            if (new_dist < cur_dist) {
                p = p_;
            }
        }

        return p;
    }

    /**
     * @brief Width of the polygon.
     * @return Width as type T.
     */
    T width() const { return br().x - bl().x; }

    /**
     * @brief Height of the polygon.
     * @return Height as type T.
     */
    T height() const { return bl().y - tl().y; }

    /**
     * @brief Bounding rectangle.
     * @return Rectangle as polygon with type T.
     */
    Polygon bounding_rect() const {
        T min_x = 0, max_x = 0;
        T min_y = 0, max_y = 0;

        min_x = max_x = m_points[0].x;
        min_y = max_y = m_points[0].y;

        for (const auto &p : m_points) {
            if (min_x > p.x) {
                min_x = p.x;
            } else if (max_x < p.x) {
                max_x = p.x;
            }
            if (min_y > p.y) {
                min_y = p.y;
            } else if (max_y < p.y) {
                max_y = p.y;
            }
        }

        // bl, tl, tr, br
        return Polygon(Point2<T>(min_x, max_y), Point2<T>(min_x, min_y), Point2<T>(max_x, min_y),
                       Point2<T>(max_x, max_y));
    }

private:
    /// Points defining the polygon in 2D (euclidian) space.
    std::vector<Point2<T>> m_points;
};

} // namespace sph

#endif // SPH_CORE_POLYGON_H
