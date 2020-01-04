/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_MATRIX_HPP
#define SPH_MATRIX_HPP

#include <cstddef>
#include <iterator>
#include <ostream>

#include "seraphim/except.hpp"

namespace sph {

template <typename T> class matrix_iterator;
template <typename T> class const_matrix_iterator;

/**
 * @brief Matrix interface.
 *
 * Implement this interface to provide matrix data access.
 * A simple implementation would store row data in DRAM for easy access.
 * More sophisticated code might make use of accelerators such as FPGAs through OpenCL or CUDA.
 */
template <typename T> class Matrix {
public:
    /**
     * @brief Memory location of a row.
     *
     * Note that array indexing is used, i.e. the first row has an index of 0.
     * The data is guaranteed to be continuous if step == cols * sizeof(T), otherwise there are
     * padding bytes at the end of each row.
     *
     * @param i The index of the matrix row.
     * @return Pointer to the matrix row.
     */
    virtual T *data(size_t i = 0) const = 0;

    /**
     * @brief Number of rows in the matrix.
     * @return Row count.
     */
    virtual size_t rows() const = 0;

    /**
     * @brief Number of columns in the matrix.
     * @return Column count.
     */
    virtual size_t cols() const = 0;

    /**
     * @brief Number of bytes per row.
     * @return Byte count.
     */
    virtual size_t step() const = 0;

    /**
     * @brief Resize the backing memory store to the specified size.
     * @param rows New row count.
     * @param cols New column count.
     */
    virtual void resize(size_t rows, size_t cols) = 0;

    /**
     * @brief operator !
     * @return True if empty, false otherwise.
     */
    virtual bool operator!() const = 0;

    /**
     * @brief operator =
     * @param value Scalar initializer.
     */
    virtual void operator=(T value) = 0;

    /**
     * @brief Subscript operator retrieving a single matrix element reference.
     *
     * Note that array indexing is used, i.e. the first element is at (0, 0).
     *
     * @param i Matrix row index.
     * @param j Matrix column index.
     * @return The Matrix element at the specified offsets.
     */
    virtual T &operator()(size_t i, size_t j) = 0;

    /**
     * @brief Subscript operator retrieving a single matrix element.
     *
     * Note that array indexing is used, i.e. the first element is at (0, 0).
     *
     * @param i Matrix row index.
     * @param j Matrix column index.
     * @return The Matrix element at the specified offsets.
     */
    T operator()(size_t i, size_t j) const {
        return (const_cast<Matrix *>(this))->operator()(i, j);
    }

    /**
     * @brief operator ==
     * @param rhs Right hand side.
     * @return True if equal, false otherwise.
     */
    bool operator==(const Matrix<T> &rhs) const { return data() == rhs.data(); }

    /**
     * @brief Begin of the matrix, points to its first element.
     *
     * May modify the current element.
     *
     * @return Forward iterator.
     */
    matrix_iterator<T> begin() { return matrix_iterator(*this, 0, 0); }

    /**
     * @brief End of the matrix, points to its last element.
     *
     * May modify the current element.
     *
     * @return Forward iterator.
     */
    matrix_iterator<T> end() { return matrix_iterator(*this, rows(), cols()); }

    /**
     * @brief Begin of the matrix, points to its first element.
     * @return Constant forward iterator.
     */
    const_matrix_iterator<T> begin() const { return const_matrix_iterator(*this, 0, 0); }

    /**
     * @brief End of the matrix, points to its last element.
     * @return Constant forward iterator.
     */
    const_matrix_iterator<T> end() const { return const_matrix_iterator(*this, rows(), cols()); }
};

template <typename T> class matrix_iterator {
public:
    typedef matrix_iterator self_type;
    typedef T value_type;
    typedef T &reference;
    typedef T *pointer;
    typedef std::forward_iterator_tag iterator_category;
    matrix_iterator(Matrix<T> &m, size_t i, size_t j) : m_mat(m), m_row(i), m_col(j) {}
    self_type operator++() {
        m_col++;
        if (m_col >= m_mat.cols()) {
            m_col = 0;
            m_row++;
        }
        if (m_col >= m_mat.cols() || m_row >= m_mat.rows()) {
            SPH_THROW(InvalidArgumentException, "m_col >= m_mat.cols() || m_row >= m_mat.rows()");
        }
        return *this;
    }
    self_type operator++(int) {
        self_type i = *this;
        m_col++;
        if (m_col >= m_mat.cols()) {
            m_col = 0;
            m_row++;
        }
        if (m_col >= m_mat.cols() || m_row >= m_mat.rows()) {
            SPH_THROW(InvalidArgumentException, "m_col >= m_mat.cols() || m_row >= m_mat.rows()");
        }
        return i;
    }
    value_type &operator*() { return m_mat(m_row, m_col); }
    value_type *operator->() { return m_mat.data(m_row) + m_col; }
    bool operator==(const self_type &rhs) { return m_mat == rhs.m_mat; }
    bool operator!=(const self_type &rhs) { return !(m_mat == rhs.m_mat); }

private:
    Matrix<T> &m_mat;
    size_t m_row;
    size_t m_col;
};

template <typename T> class const_matrix_iterator {
public:
    typedef const_matrix_iterator self_type;
    typedef T value_type;
    typedef T &reference;
    typedef T *pointer;
    typedef std::forward_iterator_tag iterator_category;
    const_matrix_iterator(const Matrix<T> &m, size_t i, size_t j) : m_mat(m), m_row(i), m_col(j) {}
    self_type operator++() {
        m_col++;
        if (m_col >= m_mat.cols()) {
            m_col = 0;
            m_row++;
        }
        if (m_col >= m_mat.cols() || m_row >= m_mat.rows()) {
            SPH_THROW(InvalidArgumentException, "m_col >= m_mat.cols() || m_row >= m_mat.rows()");
        }
        return *this;
    }
    self_type operator++(int) {
        self_type i = *this;
        m_col++;
        if (m_col >= m_mat.cols()) {
            m_col = 0;
            m_row++;
        }
        if (m_col >= m_mat.cols() || m_row >= m_mat.rows()) {
            SPH_THROW(InvalidArgumentException, "m_col >= m_mat.cols() || m_row >= m_mat.rows()");
        }
        return i;
    }
    const value_type &operator*() { return m_mat(m_row, m_col); }
    const value_type *operator->() { return m_mat.data(m_row) + m_col; }
    bool operator==(const self_type &rhs) { return m_mat == rhs.m_mat; }
    bool operator!=(const self_type &rhs) { return !(m_mat == rhs.m_mat); }

private:
    const Matrix<T> &m_mat;
    size_t m_row;
    size_t m_col;
};

/**
 * @brief Ostream operator.
 *        Prints the matrix elements organized as rows.
 * @param os Source ostream.
 * @param m Matrix instance.
 * @return Modified ostream.
 */
template <typename T> std::ostream &operator<<(std::ostream &os, Matrix<T> &m) {
    os << std::endl << "[";
    for (size_t i = 0; i < m.rows(); i++) {
        if (i > 0) {
            os << " ";
        }
        os << "[";
        for (size_t j = 0; j < m.cols(); j++) {
            os << m(i, j);
            if (j < m.cols() - 1) {
                os << " ";
            }
        }
        os << "]";
        if (i < m.rows() - 1) {
            os << std::endl;
        }
    }
    os << "]";

    return os;
}

} // namespace sph

#include "matrix/matrix_core.hpp"

#endif // SPH_MATRIX_HPP
