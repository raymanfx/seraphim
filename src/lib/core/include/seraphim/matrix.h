/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_MATRIX_H
#define SPH_CORE_MATRIX_H

#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

namespace sph {

/**
 * @brief Matrix class representing arbitrary data.
 *        Element data from external sources can either be copied or wrapped (zero-copy).
 *        Refer to the specific documentation for each constructor to see where a deep copy is
 *        performed and where a shallow copy is done.
 *
 *        Inspired by OpenCV's cv::Mat.
 */
template <typename T> class Matrix {
public:
    /**
     * @brief Default constructor for an empty matrix.
     * Use this to create instances to assign them later.
     */
    Matrix() {}

    /**
     * @brief Allocate a new matrix.
     * @param rows Number of rows.
     * @param cols Number of columns.
     */
    Matrix(size_t rows, size_t cols, size_t step = 0) : m_rows(rows), m_cols(cols), m_step(step) {
        if (m_step == 0) {
            m_step = cols * sizeof(T);
        }

        m_capacity = rows * (m_step / sizeof(T));
        m_buffer.reset(new T[m_capacity]);
        m_data = m_buffer.get();
    }

    /**
     * @brief Assign elements from an arbitrary source.
     * @param elements Pointer to elements source.
     * @param rows Number of source rows.
     * @param cols Number of source columns.
     * @param step Number of bytes per row. If 0, this is calculated as rows * sizeof(T).
     * @param transfer Whether to transfer element ownership to the newly created instance.
     */
    Matrix(T *elements, size_t rows, size_t cols, size_t step = 0, const bool &transfer = false)
        : m_rows(rows), m_cols(cols), m_step(step) {
        if (m_step == 0) {
            m_step = cols * sizeof(T);
        }

        if (transfer) {
            m_capacity = rows * (m_step / sizeof(T));
        }

        m_data = elements;
    }

    template <size_t rows, size_t cols>
    /**
     * @brief Copy matrix elements from a two dimensional array living on the stack.
     */
    Matrix(T (&elements)[rows][cols]) : Matrix(rows, cols) {
        Matrix(&elements[0][0], m_rows, m_cols).copy(*this);
    }

    /**
     * @brief Constructor that copies elements from a vector.
     *        Can be used to create element arrays in place.
     * @param elements Two dimensional vector holding elements.
     */
    Matrix(const std::vector<std::vector<T>> &elements)
        : Matrix(elements.size(), elements[0].size()) {
        // a vector is not guaranteed (unlikely even) to store its data in one contiguous block,
        // so we have to resort to per-row copying
        for (size_t i = 0; i < m_rows; i++) {
            std::copy(elements[i].begin(), elements[i].end(), m_data + i * m_cols);
        }
    }

    /**
     * @brief Construct a new matrix by cloning a region of another matrix.
     * @param m Other matrix to copy elements from.
     * @param i Row offset.
     * @param j Column offset.
     * @param rows Number of rows to copy.
     * @param cols Number of columns to copy.
     */
    Matrix(const Matrix m, size_t i, size_t j, size_t rows, size_t cols) : Matrix(rows, cols) {
        assert((i + rows) <= m.rows() && (j + cols) <= m.cols());
        for (size_t i_ = 0; i_ < rows; i_++) {
            for (size_t j_ = 0; j_ < cols; j_++) {
                m_data[i_ * m_cols + j_] = m[i_ + i][j_ + j];
            }
        }
    }

    /**
     * @brief Copy constructor, performs a deep copy of elements.
     * @param m Instance to copy.
     */
    Matrix(const Matrix &m) : Matrix(m.rows(), m.cols()) { m.copy(*this); }

    /**
     * @brief Move constructor, moves another matrices elements.
     * @param m Instance to move from.
     */
    Matrix(Matrix &&m) : Matrix() { m.move(*this); }

    ~Matrix() { clear(); }

    /**
     * @brief Copy assignment operator, performs a deep copy of elements.
     *        The step is normalized during the copy operation, i.e. step = cols * sizeof(T).
     * @param m Instance to copy from.
     * @return Current instance.
     */
    Matrix &operator=(const Matrix &m) {
        if (this != &m) {
            clear();
            m.copy(*this);
        }
        return *this;
    }

    /**
     * @brief Move assignment operator, moves the arguments' elements.
     * @param m Instance to move from.
     * @return Current instance.
     */
    Matrix &operator=(Matrix &&m) {
        if (this != &m) {
            clear();
            m.move(*this);
        }
        return *this;
    }

    /**
     * @brief Array subscript operator, returns the memory location of a row.
     *        Note that array indexing is used, i.e. the first row has an index of 0.
     * @param i The index of the matrix row.
     * @return Pointer to the matrix row.
     */
    T *operator[](size_t i) const {
        assert(i < m_rows);
        return m_data + i * (m_step / sizeof(T));
    }

    /**
     * @brief Subscript operator retrieving a single matrix element reference.
     *        Note that element indexing is used, i.e. the first element is at (1, 1).
     * @param i Matrix row index.
     * @param j Matrix column index.
     * @return The Matrix element at the specified offsets.
     */
    T &operator()(size_t i, size_t j) {
        assert(i > 0 && j > 0);
        assert(i <= m_rows && j <= m_cols);
        return (m_data + (i - 1) * (m_step / sizeof(T)))[j - 1];
    }

    /**
     * @brief Subscript operator for extracting matrix regions.
     * @param i Matrix row index.
     * @param j Matrix column index.
     * @param rows Number of rows to copy.
     * @param cols Number of columns to copy.
     * @return The new matrix obtained from the extracted region.
     */
    Matrix operator()(size_t i, size_t j, size_t rows, size_t cols) {
        assert((i + rows) <= m_rows && (j + cols) <= m_cols);
        return Matrix(*this, i, j, rows, cols);
    }

    /**
     * @brief Ostream operator.
     *        Prints the matrix elements organized as rows.
     * @param os Source ostream.
     * @param m Matrix instance.
     * @return Modified ostream.
     */
    friend std::ostream &operator<<(std::ostream &os, Matrix &m) {
        os << std::endl << "[";
        for (size_t i = 0; i < m.rows(); i++) {
            if (i > 0) {
                os << " ";
            }
            os << "[";
            for (size_t j = 0; j < m.cols(); j++) {
                os << m[i][j];
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

    /**
     * @brief Number of rows in the matrix.
     * @return Row count.
     */
    size_t rows() const { return m_rows; }

    /**
     * @brief Number of columns in the matrix.
     * @return Column count.
     */
    size_t cols() const { return m_cols; }

    /**
     * @brief Number of bytes per row.
     * @return Byte count.
     */
    size_t step() const { return m_step; }

    /**
     * @brief Number of elements.
     * @return Byte count.
     */
    size_t size() const { return m_rows * m_cols; }

    /**
     * @brief Number of elements that the matrix instance can hold in its backing buffer.
     * @return Number of elements or 0 for shallow (wrapping) matrices.
     */
    size_t capacity() const { return m_capacity; }

    /**
     * @brief Read-only pointer to matrix memory.
     * @return Memory location of the first element.
     */
    unsigned char *bytes() const { return reinterpret_cast<unsigned char *>(m_data); }

    /**
     * @brief Read-only pointer to matrix elements.
     * @return Memory location of the first element.
     *         The memory space is continuous, but padding may lead to empty or invalid elements.
     */
    T *data() const { return m_data; }

    /**
     * @brief Check whether the matrix is empty.
     *        An allocated matrix is empty by default until elements are assigned.
     * @return True if no elements are stored, false otherwise.
     */
    bool empty() const { return m_data == nullptr; }

    /**
     * @brief Clear the matrix, removing all its elementes and freeing memory.
     */
    void clear() {
        m_rows = 0;
        m_cols = 0;
        m_step = 0;
        m_data = nullptr;
        m_buffer.reset(nullptr);
        m_capacity = 0;
    }

    /**
     * @brief Reserve backing memory amount.
     *        A reallocation is guaranteed to only happen if the requested capacity is greater than
     *        the current one.
     * @param rows Number of rows.
     * @param cols Number of columns.
     * @param step Number of bytes per row. If 0, this is calculated as cols * sizeof(T).
     */
    void reserve(size_t rows, size_t cols, size_t step = 0) {
        if (step == 0) {
            step = cols * sizeof(T);
        }

        if (rows == 0 || cols == 0) {
            return;
        }

        if (m_capacity > 0 && (rows * step <= m_rows * m_step)) {
            return;
        }

        m_step = step;
        m_capacity = rows * (step / sizeof(T));
        m_buffer.reset(new T[m_capacity]);
        m_data = m_buffer.get();
    }

    /**
     * @brief Resize the backing memory store to the specified size.
     *        Causes a reallocation if the current size does not match the requested one.
     * @param rows Number of rows.
     * @param cols Number of columns.
     * @param step Number of bytes per row. If 0, this is calculated as cols * sizeof(T).
     */
    void resize(size_t rows, size_t cols, size_t step = 0) {
        if (step == 0) {
            step = cols * sizeof(T);
        }

        if (m_capacity > 0 && (rows * cols == m_rows * m_cols && step == m_step)) {
            return;
        }

        clear();
        reserve(rows, cols, step);
        m_rows = rows;
        m_cols = cols;
        m_step = step;
    }

    /**
     * @brief Reshape the matrix.
     * @param rows Number of rows.
     * @param cols Number of columns.
     */
    void reshape(size_t rows, size_t cols) {
        assert(rows * cols <= m_rows * m_cols);
        m_rows = rows;
        m_cols = cols;
        m_step = cols * sizeof(T);
    }

    /**
     * @brief Transpose the matrix.
     *
     * Very primitive approach, possible target for future optimization.
     */
    Matrix transposed() {
        Matrix t(*this);

        // reshape to target dimensions
        t.reshape(m_cols, m_rows);

        for (size_t i = 1; i <= t.rows(); i++) {
            for (size_t j = 1; j <= t.cols(); j++) {
                t(i, j) = (*this)(j, i);
            }
        }

        return t;
    }

    /**
     * @brief Copy matrix elements into another matrix.
     *        Each element of the current instance is copied into the new instance.
     * @param target Target instance which assumes element ownership.
     */
    void copy(Matrix &target) const {
        target.resize(m_rows, m_cols);

        // if the source data is continuous, we can perform an optimized copy
        if (m_step == m_cols * sizeof(T)) {
            std::copy(m_data, m_data + m_rows * m_cols, target.m_data);
            return;
        }

        // otherwise, we have to fallback to row copying (which works because padding is only ever
        // present at the end of a row, so we can just copy the data and leave out the padding)
        for (size_t i = 0; i < m_rows; i++) {
            std::copy(bytes() + i * m_step, bytes() + i * m_step + m_cols * sizeof(T),
                      target.m_data + i * target.m_cols);
        }
    }

    /**
     * @brief Move matrix elements into another matrix.
     *        This is a zero-copy operation. If memory was allocated, the other instance now owns
     *        that memory. Otherwise, it owns the pointer to the arbitrary element source.
     * @param target Target instance which assumes element ownership.
     */
    void move(Matrix &target) {
        target.m_rows = m_rows;
        target.m_cols = m_cols;
        target.m_step = m_step;
        target.m_capacity = m_capacity;
        target.m_data = m_data;
        m_buffer.swap(target.m_buffer);

        m_capacity = 0;
        clear();
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
    /// In case of wrapped data, this is null.
    std::unique_ptr<T[]> m_buffer = nullptr;
    /// Number of elements the backing buffer can hold.
    size_t m_capacity = 0;
};

} // namespace sph

#endif // SPH_CORE_MATRIX_H
