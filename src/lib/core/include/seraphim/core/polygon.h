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

namespace sph {
namespace core {

/**
 * @brief Polygon object in 2D (euclidian) space.
 * By default, the type of the coordinates is integer.
 */
template <typename T = int> class Polygon {
public:
    /**
     * @brief Point coordinates defining a point in 2D (euclidian) space.
     */
    struct Point {
        /// x coordinate
        T x;
        /// y coordinate
        T y;
    };

    /**
     * @brief Default constructor for an empty polygon.
     * Points have to be added manually via @ref add_point.
     */
    Polygon() = default;
    /**
     * @brief Create a new polygon shape.
     * @param list List of points defining the polygon.
     */
    Polygon(std::initializer_list<Point> list) {
        for (const auto &entry : list) {
            m_points.push_back(entry);
        }
    }

    /**
     * @brief Check whether the polygon is empty (i.e. undefined).
     * @return True if there are less than three points, false otherwise.
     */
    bool empty() { return m_points.size() < 3; }
    /**
     * @brief Clear the polygon, removing all its points.
     */
    void clear() { m_points.clear(); }

    /**
     * @brief Get the points that make the polygon shape.
     * @return Set of points in order of appearance.
     */
    std::vector<Point> points() const { return m_points; }
    /**
     * @brief Add a point to the polygon.
     * @param p Point with type T.
     */
    void add_point(const Point &p) { m_points.push_back(p); }

    /**
     * @brief Bottom left point.
     * @return Point as set of coordinates with type T.
     */
    Point bl() const {
        T smallest_x;
        T smallest_y;

        if (m_points.size() == 0) {
            return { smallest_x, smallest_y };
        }

        smallest_x = m_points[0].x;
        smallest_y = m_points[0].y;

        for (const auto &p : m_points) {
            if (smallest_x > p.x) {
                smallest_x = p.x;
            }
            if (smallest_y > p.y) {
                smallest_y = p.y;
            }
        }

        return { smallest_x, smallest_y };
    }
    /**
     * @brief Top left point.
     * @return Point as set of coordinates with type T.
     */
    Point tl() const {
        T smallest_x;
        T largest_y;

        if (m_points.size() == 0) {
            return { smallest_x, largest_y };
        }

        smallest_x = m_points[0].x;
        largest_y = m_points[0].y;

        for (const auto &p : m_points) {
            if (smallest_x > p.x) {
                smallest_x = p.x;
            }
            if (largest_y < p.y) {
                largest_y = p.y;
            }
        }

        return { smallest_x, largest_y };
    }
    /**
     * @brief Top right point.
     * @return Point as set of coordinates with type T.
     */
    Point tr() const {
        T largest_x;
        T largest_y;

        if (m_points.size() == 0) {
            return { largest_x, largest_y };
        }

        largest_x = m_points[0].x;
        largest_y = m_points[0].y;

        for (const auto &p : m_points) {
            if (largest_x < p.x) {
                largest_x = p.x;
            }
            if (largest_y < p.y) {
                largest_y = p.y;
            }
        }

        return { largest_x, largest_y };
    }
    /**
     * @brief Bottom right point.
     * @return Point as set of coordinates with type T.
     */
    Point br() const {
        T largest_x;
        T smallest_y;

        if (m_points.size() == 0) {
            return { largest_x, smallest_y };
        }

        largest_x = m_points[0].x;
        smallest_y = m_points[0].y;

        for (const auto &p : m_points) {
            if (smallest_y > p.y) {
                smallest_y = p.y;
            }
            if (largest_x < p.x) {
                largest_x = p.x;
            }
        }

        return { largest_x, smallest_y };
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
    T height() const { return tl().y - bl().y; }
    /**
     * @brief Bounding rectangle.
     * @return Rectangle as polygon with type T.
     */
    Polygon<T> bounding_rect() const;

private:
    /// Points defining the polygon in 2D (euclidian) space.
    std::vector<Point> m_points;
};

} // namespace core
} // namespace sph

#endif // SPH_CORE_POLYGON_H
