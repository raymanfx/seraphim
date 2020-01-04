/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_MATRIX_CORE_H
#define SPH_MATRIX_CORE_H

#include <cassert>
#include <cmath>
#include <memory>
#include <vector>

#include "seraphim/except.h"

namespace sph {

template <typename T> class Matrix;

/**
 * @brief Matrix class representing arbitrary data.
 *
 * Element data from external sources can either be copied or wrapped (zero-copy).
 * Refer to the specific documentation for each constructor to see where a deep copy is
 * performed and where a shallow copy is done.
 */
template <typename T> class CoreMatrix : public Matrix<T> {
public:
    /**
     * @brief Default constructor for an empty matrix.
     * Use this to create instances to assign them later.
     */
    CoreMatrix() {}

    /**
     * @brief Allocate a new matrix.
     * @param rows Number of rows.
     * @param cols Number of columns.
     */
    CoreMatrix(size_t rows, size_t cols) : m_rows(rows), m_cols(cols) {
        m_step = cols * sizeof(T);
        m_buffer.resize(rows * cols);
        m_data = m_buffer.data();
    }

    /**
     * @brief Assign elements from an arbitrary source.
     * @param elements Pointer to elements source.
     * @param rows Number of source rows.
     * @param cols Number of source columns.
     * @param step Number of bytes per row. If 0, this is calculated as rows * sizeof(T).
     */
    CoreMatrix(T *elements, size_t rows, size_t cols, size_t step = 0)
        : m_rows(rows), m_cols(cols), m_step(step) {
        if (m_step == 0) {
            m_step = cols * sizeof(T);
        }

        m_data = elements;
    }

    /**
     * @brief Transform a matrix into a RAM-backed matrix.
     * @param mat Input matrix.
     */
    template <typename _T> CoreMatrix(const Matrix<_T> &mat) {
        if constexpr (std::is_same_v<_T, T>) {
            *this = CoreMatrix(mat.data(), mat.rows(), mat.cols(), mat.step());
        } else {
            *this = CoreMatrix(mat.rows(), mat.cols());
            for (size_t i = 0; i < mat.rows(); i++) {
                for (size_t j = 0; j < mat.cols(); j++) {
                    (*this)(i, j) = mat(i, j);
                }
            }
        }
    }

    /**
     * @brief Copy matrix elements from a two dimensional array living on the stack.
     */
    template <size_t rows, size_t cols>
    CoreMatrix(T (&elements)[rows][cols]) : CoreMatrix(&elements[0][0], rows, cols) {}

    /**
     * @brief Constructor that copies elements from a vector.
     *        Can be used to create element arrays in place.
     * @param elements Two dimensional vector holding elements.
     */
    CoreMatrix(const std::vector<std::vector<T>> &elements)
        : CoreMatrix(elements.size(), elements[0].size()) {
        // a vector is not guaranteed (unlikely even) to store its data in one contiguous block,
        // so we have to resort to per-row copying
        for (size_t i = 0; i < m_rows; i++) {
            std::copy(elements[i].begin(), elements[i].end(), m_data + i * m_cols);
        }
    }

    /**
     * @brief Copy constructor, performs a deep copy of elements.
     * @param m Instance to copy.
     */
    CoreMatrix(const CoreMatrix &m) : CoreMatrix(m.rows(), m.cols()) {
        resize(m.rows(), m.cols());

        // if the source data is continuous, we can perform an optimized copy
        if (m_step == m_cols * sizeof(T)) {
            std::copy(m.data(), m.data() + m.rows() * m.cols(), m_data);
            return;
        }

        // otherwise, we have to fallback to row copying (which works because padding is only ever
        // present at the end of a row, so we can just copy the data and leave out the padding)
        auto src_data = reinterpret_cast<unsigned char *>(m.m_data);
        auto dst_data = reinterpret_cast<unsigned char *>(m_data);
        for (size_t i = 0; i < m.rows(); i++) {
            std::copy(src_data + i * m.step(), src_data + i * m.step() + m.cols() * sizeof(T),
                      dst_data + i * m.cols());
        }
    }

    /**
     * @brief Move constructor, moves another matrices elements.
     * @param m Instance to move from.
     */
    CoreMatrix(CoreMatrix &&m) : CoreMatrix() {
        m_rows = m.m_rows;
        m_cols = m.m_cols;
        m_step = m.m_step;
        m_data = m.m_data;
        m_buffer = std::move(m.m_buffer);

        m.m_rows = 0;
        m.m_cols = 0;
        m.m_step = 0;
        m.m_data = nullptr;
    }

    /**
     * @brief Copy assignment operator, performs a deep copy of elements.
     *        The step is normalized during the copy operation, i.e. step = cols * sizeof(T).
     * @param m Instance to copy from.
     * @return Current instance.
     */
    CoreMatrix &operator=(const CoreMatrix<T> &m) {
        if (this != &m) {
            CoreMatrix<T> tmp(m);
            m_rows = tmp.m_rows;
            m_cols = tmp.m_cols;
            m_step = tmp.m_step;
            m_data = tmp.m_data;

            tmp.m_buffer.swap(m_buffer);
        }
        return *this;
    }

    /**
     * @brief Move assignment operator, moves the arguments' elements.
     * @param m Instance to move from.
     * @return Current instance.
     */
    CoreMatrix &operator=(CoreMatrix &&m) {
        if (this != &m) {
            m_rows = m.m_rows;
            m_cols = m.m_cols;
            m_step = m.m_step;
            m_data = m.m_data;
            m_buffer = std::move(m.m_buffer);

            m.m_rows = 0;
            m.m_cols = 0;
            m.m_step = 0;
            m.m_data = nullptr;
        }
        return *this;
    }

    // Matrix<T> interface ////////////////////////////////////////////////////////////////////////

    T *data(size_t i = 0) const override {
        if (i > 0) {
            assert(i < m_rows);
        }
        return reinterpret_cast<T *>(reinterpret_cast<std::byte *>(m_data) + i * m_step);
    }

    size_t rows() const override { return m_rows; }
    size_t cols() const override { return m_cols; }
    size_t step() const override { return m_step; }

    void resize(size_t rows, size_t cols) override {
        if (m_buffer.capacity() > 0 && (rows * cols == m_rows * m_cols)) {
            return;
        }

        m_rows = rows;
        m_cols = cols;
        m_step = cols * sizeof(T);
        m_buffer.resize(m_rows * m_step);
        m_data = m_buffer.data();
    }

    bool operator!() const override { return m_data == nullptr; }

    void operator=(T value) override {
        resize(m_cols, m_rows);
        m_buffer.assign(m_buffer.size(), value);
    }

    T &operator()(size_t i, size_t j) override {
        assert(i < m_rows && j < m_cols);
        return data(i)[j];
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    using sph::Matrix<T>::operator();

    /**
     * @brief Subscript operator retrieving a matrix region.
     *
     * Note that array indexing is used, i.e. the first element is at (0, 0).
     *
     * @param i Matrix row index.
     * @param j Matrix column index.
     * @param rows Number of rows to copy.
     * @param cols Number of cols to copy.
     * @return The Matrix region with copied elements.
     */
    CoreMatrix operator()(size_t i, size_t j, size_t rows, size_t cols) const {
        assert((i + rows) <= m_rows && (j + cols) <= m_cols);

        CoreMatrix tmp(rows, cols);
        for (size_t i_ = 0; i_ < rows; i_++) {
            for (size_t j_ = 0; j_ < cols; j_++) {
                tmp(i_, j_) = Matrix<T>::operator()(i_ + i, j_ + j);
            }
        }

        return tmp;
    }

    /**
     * @brief Clone a matrix instance.
     * The cloned instance will own a deep copy of the data of the current instance.
     * @return New instance with copied elements.
     */
    inline CoreMatrix clone() const {
        CoreMatrix tmp(*this);
        return tmp;
    }

    /**
     * @brief Eliminate padding in the data.
     * Will copy the buffer contents if not already owned.
     */
    void pack() {
        // Nothing to do if there is no padding.
        if (step() == cols() * sizeof(T)) {
            return;
        }

        // If we are just wrapping external data, copy it so we own it.
        // The copy operation will automatically enforce step == cols * sizeof(T), meaning it will
        // elimate any padding.
        *this = this->clone();
    }

private:
    /// Matrix rows.
    size_t m_rows = 0;

    /// Matrix columns.
    size_t m_cols = 0;

    /// Matrix step (to account for padding), eqivalent to image stride.
    size_t m_step = 0;

    /// Matrix element pointer (can be arbitrary source or internal buffer).
    T *m_data = nullptr;

    /// Back buffer, only valid if the instance has allocated memory.
    /// In case of wrapped data, this is empty.
    std::vector<T> m_buffer;
};

} // namespace sph

#include "matrix_core_conv.h"
#include "matrix_core_ops.h"

#endif // SPH_MATRIX_CORE_H
