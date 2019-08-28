/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_MATRIX_H
#define SPH_CORE_MATRIX_H

#include <cassert>
#include <iostream>
#include <vector>

namespace sph {
namespace core {

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
    Matrix() = default;

    /**
     * @brief Allocate a new matrix.
     * @param rows Number of rows.
     * @param cols Number of columns.
     */
    Matrix(const size_t &rows, const size_t &cols, const size_t &step = 0)
        : m_rows(rows), m_cols(cols), m_step(step) {
        if (m_step == 0) {
            m_step = cols * sizeof(T);
        }
        m_elements = new T[rows * m_step / sizeof(T)];
        m_elements_owned = true;
        m_elements_capacity = rows * m_step / sizeof(T);
    }

    /**
     * @brief Assign elements from an arbitrary source.
     * @param elements Pointer to elements source.
     * @param rows Number of source rows.
     * @param cols Number of source columns.
     * @param step Number of bytes per row. If 0, this is calculated as rows * sizeof(T).
     * @param transfer Whether to transfer element ownership to the newly created instance.
     */
    Matrix(T *elements, const size_t &rows, const size_t &cols, const size_t &step = 0,
           const bool &transfer = false)
        : m_rows(rows), m_cols(cols), m_step(step), m_elements(elements),
          m_elements_owned(transfer) {
        if (m_step == 0) {
            m_step = cols * sizeof(T);
        }
        if (m_elements_owned) {
            m_elements_capacity = rows * cols;
        }
    }

    template <size_t rows, size_t cols>
    /**
     * @brief Copy matrix elements from a two dimensional array living on the stack.
     */
    Matrix(T (&elements)[rows][cols]) : m_rows(rows), m_cols(cols), m_step(cols * sizeof(T)) {
        m_elements = new T[rows * cols];
        m_elements_owned = true;
        m_elements_capacity = rows * cols;

        for (size_t i = 0; i < rows; i++) {
            for (size_t j = 0; j < cols; j++) {
                m_elements[i * cols + j] = elements[i][j];
            }
        }
    }

    /**
     * @brief Constructor that copies elements from a vector.
     *        Can be used to create element arrays in place.
     * @param elements Two dimensional vector holding elements.
     */
    Matrix(const std::vector<std::vector<T>> &elements)
        : m_rows(elements.size()), m_cols(elements[0].size()), m_step(m_cols * sizeof(T)) {
        m_elements = new T[m_rows * m_cols];
        m_elements_owned = true;
        m_elements_capacity = m_rows * m_cols;

        for (size_t i = 0; i < m_rows; i++) {
            for (size_t j = 0; j < m_cols; j++) {
                m_elements[i * m_cols + j] = elements[i][j];
            }
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
    Matrix(const Matrix m, const size_t &i, const size_t &j, const size_t &rows, const size_t &cols)
        : Matrix(rows, cols) {
        assert((i + rows) <= m.rows() && (j + cols) <= m.cols());
        for (size_t i_ = 0; i_ < rows; i_++) {
            for (size_t j_ = 0; j_ < cols; j_++) {
                m_elements[i_ * m_cols + j_] = m[i_ + i][j_ + j];
            }
        }
    }

    /**
     * @brief Copy constructor, performs a deep copy of elements.
     * @param m Instance to copy.
     */
    Matrix(const Matrix &m) : Matrix(m.rows(), m.cols()) {
        for (size_t i = 0; i < m.rows(); i++) {
            for (size_t j = 0; j < m.cols(); j++) {
                m_elements[i * m_cols + j] = m[i][j];
            }
        }
    }

    ~Matrix() { clear(); }

    /**
     * @brief Copy assignment operator, performs a shallow copy of elements.
     * @param m Instance to copy from.
     * @return Current instance.
     */
    Matrix &operator=(const Matrix &m) {
        if (this != &m) {
            clear();

            m_elements = m.data();
            m_rows = m.rows();
            m_cols = m.cols();
            m_step = m.step();
        }
        return *this;
    }

    /**
     * @brief Array subscript operator, returns the memory location of a row.
     * @param i The index of the matrix row.
     * @return Pointer to the matrix row.
     */
    T *operator[](const size_t &i) const {
        assert(i < m_rows);
        return m_elements + i * (m_step / sizeof(T));
    }

    /**
     * @brief Subscript operator for extracting matrix regions.
     * @param i Matrix row index.
     * @param j Matrix column index.
     * @param rows Number of rows to copy.
     * @param cols Number of columns to copy.
     * @return The new matrix obtained from the extracted region.
     */
    Matrix operator()(const size_t &i, const size_t &j, const size_t &rows, const size_t &cols) {
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
     * @brief Number of bytes occupied by the matrix.
     *        This is calculated as the number of rows times the number of bytes per row.
     * @return Byte count.
     */
    size_t size() const { return m_rows * m_step; }

    /**
     * @brief Read-only pointer to matrix memory.
     * @return Memory location of the first element.
     */
    unsigned char *bytes() const { return reinterpret_cast<unsigned char *>(m_elements); }

    /**
     * @brief Read-only pointer to matrix elements.
     * @return Memory location of the first element.
     *         The memory space is continuous, but padding may lead to empty or invalid elements.
     */
    T *data() const { return m_elements; }

    /**
     * @brief Check whether the matrix owns its element backing memory.
     * @return True if the memory is owned by the instance, false otherwise.
     */
    bool owns_data() const { return m_elements_owned; }

    /**
     * @brief Check whether the matrix is empty.
     *        An allocated matrix is empty by default until elements are assigned.
     * @return True if no elements are stored, false otherwise.
     */
    bool empty() const { return !m_elements; }

    /**
     * @brief Clear the matrix, removing all its elementes and freeing memory.
     */
    void clear() {
        m_rows = 0;
        m_cols = 0;
        m_step = 0;
        if (m_elements_owned && m_elements) {
            delete[] m_elements;
        }
        m_elements = nullptr;
        m_elements_owned = false;
        m_elements_capacity = 0;
    }

    /**
     * @brief Reserve backing memory amount.
     *        A reallocation is guaranteed to only happen if the requested capacity is greater than
     *        the current one.
     * @param rows Number of rows.
     * @param cols Number of columns.
     */
    void reserve(const size_t &rows, const size_t &cols) {
        if (rows * cols <= m_elements_capacity) {
            return;
        }

        clear();
        m_step = cols * sizeof(T);
        m_elements = new T[rows * cols];
        m_elements_owned = true;
        m_elements_capacity = rows * cols;
    }

    /**
     * @brief Resize the backing memory store to the specified size.
     *        Causes a reallocation if the current size does not match the requested one.
     * @param rows Number of rows.
     * @param cols Number of columns.
     */
    void resize(const size_t &rows, const size_t &cols) {
        if (rows * cols == m_rows * m_cols) {
            return;
        }

        clear();
        reserve(rows, cols);
        m_rows = rows;
        m_cols = cols;
        m_step = cols * sizeof(T);
    }

    /**
     * @brief Reshape the matrix.
     * @param rows Number of rows.
     * @param cols Number of columns.
     */
    void reshape(const size_t &rows, const size_t &cols) {
        assert(rows * cols <= m_rows * m_cols);
        m_rows = rows;
        m_cols = cols;
        m_step = cols * sizeof(T);
    }

    /**
     * @brief Copy matrix elements into another matrix.
     *        Each element of the current instance is copied into the new instance.
     * @param target Target instance which assumes element ownership.
     */
    void copy(Matrix &target) {
        target.resize(m_rows, m_cols);
        for (size_t i = 0; i < m_rows; i++) {
            for (size_t j = 0; j < m_cols; j++) {
                target[i][j] = (*this)[i][j];
            }
        }
    }

    /**
     * @brief Move matrix elements into another matrix.
     *        This is a zero-copy operation. If memory was allocated, the other instance now owns
     *        that memory. Otherwise, it owns the pointer to the arbitrary element source.
     * @param target Target instance which assumes element ownership.
     */
    void move(Matrix &target) {
        target = Matrix(m_elements, m_rows, m_cols, m_step, m_elements_owned);
        m_elements_owned = false;
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
    T *m_elements = nullptr;
    /// Whether we own the element data.
    bool m_elements_owned = false;
    /// Number of matrix elements the backing buffer can hold.
    size_t m_elements_capacity = 0;
};

} // namespace core
} // namespace sph

#endif // SPH_CORE_MATRIX_H
