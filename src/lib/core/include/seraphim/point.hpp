/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_POINT_HPP
#define SPH_CORE_POINT_HPP

#include <array>
#include <cassert>
#include <vector>

namespace sph {

/**
 * @brief Point in n-dimensional space.
 */
template <typename T, size_t N> class Point {
public:
    /**
     * @brief Default constructor for an empty point.
     * To set its coordinates, use the array subscript operator.
     */
    Point() = default;

    /**
     * @brief Constructor for a n-dimensional point.
     * At least one argument must be given and all arguments must be of the same type.
     */
    template <typename... Ts,
              typename std::enable_if<(sizeof...(Ts) == N && N > 0)>::type * = nullptr>
    Point(Ts... coords) : m_coords{ coords... } {}

    /**
     * @brief Array subscript operator.
     * @param dim The dimension you want.
     * @return The coordinate in the specified dimension.
     */
    T &operator[](size_t dim) {
        assert(N > dim);
        return m_coords[dim];
    }

    /**
     * @brief operator ==
     * @param rhs Right hand side.
     * @return True if equal, false otherwise.
     */
    bool operator==(const Point &rhs) const { return m_coords == rhs.m_coords; }

    /**
     * @brief Read-only access to the coordinate list.
     * @return Fixed-size iterable array.
     */
    std::array<T, N> coords() const { return m_coords; }

protected:
    /// Coordinates of the point in n-dimensional space, where n = array size.
    std::array<T, N> m_coords;
};

/**
 * @brief Point in 2-dimensional space.
 */
template <typename T> class Point2 : public Point<T, 2> {
public:
    /**
     * @brief Default constructor for an empty point.
     */
    Point2() : Point<T, 2>(T(), T()) {}

    /**
     * @brief Constructor for a 2-dimensional point.
     */
    Point2(T x, T y) : Point<T, 2>(x, y) {}

    /**
     * @brief Copy constructor, creates a new instance and initializes x and y members.
     * @param p Instance to copy.
     */
    Point2(const Point2 &p) : Point2(p.x, p.y) {}

    /**
     * @brief Assignment operator, assigning the internal coordinate container.
     * @param p Instance to copy from.
     */
    void operator=(const Point2 &p) { this->m_coords = p.coords(); }

    /// X coordinate.
    T &x = this->m_coords[0];
    /// Y coordinate.
    T &y = this->m_coords[1];
};

/**
 * @brief Point in 3-dimensional space.
 */
template <typename T> class Point3 : public Point<T, 3> {
public:
    /**
     * @brief Default constructor for an empty point.
     */
    Point3() : Point<T, 3>(T(), T(), T()) {}

    /**
     * @brief Constructor for a 3-dimensional point.
     */
    Point3(T x, T y, T z) : Point<T, 3>(x, y, z) {}

    /**
     * @brief Copy constructor, creates a new instance and initializes x and y members.
     * @param p Instance to copy.
     */
    Point3(const Point3 &p) : Point3(p.x, p.y, p.z) {}

    /**
     * @brief Assignment operator, assigning the internal coordinate container.
     * @param p Instance to copy from.
     */
    void operator=(const Point3 &p) { this->m_coords = p.coords(); }

    /// X coordinate.
    T &x = this->m_coords[0];
    /// Y coordinate.
    T &y = this->m_coords[1];
    /// Z coordinate.
    T &z = this->m_coords[2];
};

/*
 * These are provided for convenience.
 */
using Point2i = Point2<int>;
using Point2f = Point2<float>;
using Point2d = Point2<double>;
using Point3i = Point3<int>;
using Point3f = Point3<float>;
using Point3d = Point3<double>;

} // namespace sph

#endif // SPH_CORE_POINT_HPP
