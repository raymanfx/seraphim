/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_POINT_H
#define SPH_CORE_POINT_H

#include <array>
#include <cassert>
#include <vector>

namespace sph {
namespace core {

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
    T &operator[](const size_t &dim) {
        assert(N > dim);
        return m_coords[dim];
    }

    /**
     * @brief Read-only access to the coordinate list.
     * @return Fixed-size iterable array.
     */
    std::array<T, N> coords() const { return m_coords; }

private:
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
    Point2() = default;

    /**
     * @brief Constructor for a 2-dimensional point.
     */
    Point2(const T &x, const T &y) : x(x), y(y) {}

    /// X coordinate.
    T x;
    /// Y coordinate.
    T y;
};

/**
 * @brief Point in 3-dimensional space.
 */
template <typename T> class Point3 : public Point<T, 3> {
public:
    /**
     * @brief Default constructor for an empty point.
     */
    Point3() = default;

    /**
     * @brief Constructor for a 3-dimensional point.
     */
    Point3(const T &x, const T &y, const T &z) : x(x), y(y), z(z) {}

    /// X coordinate.
    T x;
    /// Y coordinate.
    T y;
    /// Z coordinate.
    T z;
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

} // namespace core
} // namespace sph

#endif // SPH_CORE_POINT_H
