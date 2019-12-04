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

#include "seraphim/except.h"
#include "seraphim/point.h"

namespace sph {

template <typename T> class Rectangle;

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
     */
    Polygon() = default;

    /**
     * @brief Create a new polygon shape.
     * @param points List of points defining the polygon.
     */
    template <typename... Points> Polygon(Points &&... points) : m_vertices{ points... } {
        if (m_vertices.size() < 3) {
            SPH_THROW(RuntimeException, "Polygon requires at least three points");
        }
    }

    /**
     * @brief Create a new polygon shape.
     * @param points List of points defining the polygon.
     */
    template <class Container> Polygon(Container &c) : m_vertices(c.begin(), c.end()) {
        if (m_vertices.size() < 3) {
            SPH_THROW(RuntimeException, "Polygon requires at least three points");
        }
    }

    /**
     * @brief Check whether the polygon is empty (i.e. undefined).
     * @return True if there are less than three points, false otherwise.
     */
    bool empty() const { return m_vertices.empty(); }

    /**
     * @brief Get the points that make the polygon shape.
     * @return Set of points in order of appearance.
     */
    std::vector<Point2<T>> vertices() const { return m_vertices; }

    /**
     * @brief operator ==
     * @param rhs Right hand side.
     * @return True if equal, false otherwise.
     */
    bool operator==(const Polygon &rhs) const { return m_vertices == rhs.m_vertices; }

    /**
     * @brief operator !=
     * @param rhs Right hand side.
     * @return True if unequal, false otherwise.
     */
    bool operator!=(const Polygon &rhs) const { return m_vertices != rhs.m_vertices; }

    /**
     * @brief Bounding rectangle.
     * @return Rectangle as polygon with type T.
     */
    Rectangle<T> brect() const {
        T min_x = 0;
        T max_x = 0;
        T min_y = 0;
        T max_y = 0;

        // start with the first vertex if there are any
        if (m_vertices.size() > 0) {
            min_x = m_vertices[0].x;
            max_x = m_vertices[0].x;
            min_y = m_vertices[0].y;
            max_y = m_vertices[0].y;
        }

        for (const auto &p : m_vertices) {
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

        return Rectangle(Point2<T>(min_x, min_y), Point2<T>(max_x, max_y));
    }

    /**
     * @brief Width of the polygon.
     * @return Width as type T.
     */
    T width() const {
        Rectangle<T> bounding = brect();
        return bounding.br().x - bounding.bl().x;
    }

    /**
     * @brief Height of the polygon.
     * @return Height as type T.
     */
    T height() const {
        Rectangle<T> bounding = brect();
        return bounding.bl().y - bounding.tl().y;
    }

protected:
    /// Points defining the polygon in 2D (euclidian) space.
    std::vector<Point2<T>> m_vertices;
};

/**
 * Rectangle (specialization of a polygon).
 * A valid rectangle has four vertices and four edges, with the constraint that two edges must be
 * parallel to each other and no edges may intersect.
 */
template <typename T> class Rectangle : public Polygon<T> {
public:
    /**
     * @brief Default constructor for an empty rectangle.
     */
    Rectangle() = default;

    /**
     * @brief Rectangle from polygon with four vertices.
     * @param p Source polygon to transform.
     */
    Rectangle(const Polygon<T> &p) : Polygon<T>::m_vertices(p.vertices()) {
        if (Polygon<T>::m_vertices.size() != 4) {
            SPH_THROW(RuntimeException, "Rectangle requires exactly four points");
        }
    }

    /**
     * @brief Create a new rectangular shape.
     * @param p1 Edge vertex.
     * @param p2 Edge vertex.
     */
    Rectangle(const Point2<T> &p1, const Point2<T> &p2) {
        Polygon<T>::m_vertices.emplace_back(p1);
        Polygon<T>::m_vertices.emplace_back(p2);
        Polygon<T>::m_vertices.emplace_back(Point2<T>(p1.x, p2.y));
        Polygon<T>::m_vertices.emplace_back(Point2<T>(p2.x, p1.y));
    }

    /**
     * @brief Create a new rectangular shape.
     * @param p1 Top left edge vertex.
     * @param width Width of the rectangle.
     * @param height Height of the rectangle.
     */
    Rectangle(const Point2<T> &p1, T width, T height) {
        Polygon<T>::m_vertices.emplace_back(p1);
        Polygon<T>::m_vertices.emplace_back(Point2<T>(p1.x + width, p1.y));
        Polygon<T>::m_vertices.emplace_back(Point2<T>(p1.x, p1.y + height));
        Polygon<T>::m_vertices.emplace_back(Point2<T>(p1.x + width, p1.y + height));
    }

    /**
     * @brief Bottom left point.
     * @return Point as set of coordinates with type T.
     */
    Point2<T> bl() const {
        Point2<T> p = Polygon<T>::m_vertices[0];

        for (const auto &p_ : Polygon<T>::m_vertices) {
            if (p_.y >= p.y && p_.x <= p.x) {
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
        Point2<T> p = Polygon<T>::m_vertices[0];

        for (const auto &p_ : Polygon<T>::m_vertices) {
            if (p_.y <= p.y && p_.x <= p.x) {
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
        Point2<T> p = Polygon<T>::m_vertices[0];

        for (const auto &p_ : Polygon<T>::m_vertices) {
            if (p_.y <= p.y && p_.x >= p.x) {
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
        Point2<T> p = Polygon<T>::m_vertices[0];

        for (const auto &p_ : Polygon<T>::m_vertices) {
            if (p_.y >= p.y && p_.x >= p.x) {
                p = p_;
            }
        }

        return p;
    }
};

} // namespace sph

#endif // SPH_CORE_POLYGON_H
