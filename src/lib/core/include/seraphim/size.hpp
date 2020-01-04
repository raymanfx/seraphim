/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_SIZE_HPP
#define SPH_CORE_SIZE_HPP

#include <array>
#include <cassert>

namespace sph {

/**
 * @brief N-dimensional size attribute.
 */
template <typename T, size_t N> class Size {
public:
    /**
     * @brief Default constructor for an empty size.
     * To set its values, use the array subscript operator.
     */
    Size() = default;

    /**
     * @brief Constructor for a n-dimensional size.
     * At least one argument must be given and all arguments must be of the same type.
     */
    template <typename... Ts,
              typename std::enable_if<(sizeof...(Ts) == N && N > 0)>::type * = nullptr>
    Size(Ts... values) : m_values{ values... } {}

    /**
     * @brief Array subscript operator.
     * @param dim The dimension you want.
     * @return The value in the specified dimension.
     */
    T &operator[](size_t dim) {
        assert(N > dim);
        return m_values[dim];
    }

    /**
     * @brief Read-only access to the value list.
     * @return Fixed-size iterable array.
     */
    std::array<T, N> values() const { return m_values; }

    /**
     * @brief operator ==
     * @param rhs Right hand side.
     * @return True if equal, false otherwise.
     */
    bool operator==(const Size &rhs) const { return m_values == rhs.m_values; }

protected:
    /// Values of the size in n-dimensional space, where n = array size.
    std::array<T, N> m_values;
};

/**
 * @brief Size in 2-dimensional space.
 */
template <typename T> class Size2 : public Size<T, 2> {
public:
    /**
     * @brief Default constructor for an empty size.
     */
    Size2() : Size<T, 2>(T(), T()) {}

    /**
     * @brief Constructor for a 2-dimensional size.
     */
    Size2(T width, T height) : Size<T, 2>(width, height) {}

    /**
     * @brief Copy constructor, creates a new instance and initializes members.
     * @param p Instance to copy.
     */
    Size2(const Size2 &p) : Size2(p.width, p.height) {}

    /**
     * @brief Assignment operator, assigning the internal value container.
     * @param p Instance to copy from.
     */
    void operator=(const Size2 &p) { this->m_values = p.values(); }

    /// Width value.
    T &width = this->m_values[0];
    /// Height value.
    T &height = this->m_values[1];
};

/**
 * @brief Size in 3-dimensional space.
 */
template <typename T> class Size3 : public Size<T, 3> {
public:
    /**
     * @brief Default constructor for an empty size.
     */
    Size3() : Size<T, 3>(T(), T(), T()) {}

    /**
     * @brief Constructor for a 3-dimensional size.
     */
    Size3(T width, T height, T depth) : Size<T, 3>(width, height, depth) {}

    /**
     * @brief Copy constructor, creates a new instance and initializes members.
     * @param p Instance to copy.
     */
    Size3(const Size3 &p) : Size3(p.width, p.height, p.depth) {}

    /**
     * @brief Assignment operator, assigning the internal value container.
     * @param p Instance to copy from.
     */
    void operator=(const Size3 &p) { this->m_values = p.values(); }

    /// Width value.
    T &width = this->m_values[0];
    /// Height value.
    T &height = this->m_values[1];
    /// Depth value.
    T &depth = this->m_values[2];
};

/*
 * These are provided for convenience.
 */
using Size2s = Size2<size_t>;
using Size2i = Size2<int>;
using Size2f = Size2<float>;
using Size2d = Size2<double>;
using Size3s = Size3<size_t>;
using Size3i = Size3<int>;
using Size3f = Size3<float>;
using Size3d = Size3<double>;

} // namespace sph

#endif // SPH_CORE_SIZE_HPP
