/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_MATRIX_H
#define SPH_MATRIX_H

#include <cassert>
#include <cstddef>

namespace sph {

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
     * @brief operator !
     * @return True if empty, false otherwise.
     */
    virtual bool operator!() const = 0;

    /**
     * @brief Subscript operator retrieving a single matrix element reference.
     *
     * Note that array indexing is used, i.e. the first element is at (0, 0).
     *
     * @param i Matrix row index.
     * @param j Matrix column index.
     * @return The Matrix element at the specified offsets.
     */
    T operator()(size_t i, size_t j) const {
        assert(i >= 0 && j >= 0);
        assert(i < rows() && j < cols());
        return data(i)[j];
    }
};

} // namespace sph

#include "matrix/matrix_core.h"

#endif // SPH_MATRIX_H
