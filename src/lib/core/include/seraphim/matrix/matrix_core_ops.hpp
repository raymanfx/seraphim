/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_MATRIX_CORE_OPS_HPP
#define SPH_MATRIX_CORE_OPS_HPP

#include <array>
#include <iostream>
#include <vector>

#ifdef WITH_BLAS
#include <openblas/cblas.h>
#endif

#include "seraphim/except.hpp"

namespace sph {

template <typename T> class Matrix;
template <typename T> class CoreMatrix;

/**
 * @brief Transpose the matrix (rows become columns).
 * @param mat Input matrix.
 * @return Output matrix.
 */
template <typename T> CoreMatrix<T> transpose(const Matrix<T> &mat) {
    CoreMatrix<T> result(mat.cols(), mat.rows());

    for (size_t i = 0; i < mat.rows(); i++) {
        for (size_t j = 0; j < mat.cols(); j++) {
            result(j, i) = mat(i, j);
        }
    }

    return result;
}

/**
 * @brief Transpose the matrix (rows become columns).
 * @param mat Input matrix.
 * @return Output matrix.
 */
template <size_t BLOCK_SIZE = 16, typename T> CoreMatrix<T> transpose_block(const Matrix<T> &mat) {
    CoreMatrix<T> result(mat.cols(), mat.rows());
    size_t block_rows;
    size_t block_cols;

    for (size_t i = 0; i < mat.rows(); i += BLOCK_SIZE) {
        for (size_t j = 0; j < mat.cols(); j += BLOCK_SIZE) {
            block_rows = mat.rows() - i > BLOCK_SIZE ? BLOCK_SIZE : mat.rows() - i;
            block_cols = mat.cols() - j > BLOCK_SIZE ? BLOCK_SIZE : mat.cols() - j;

            for (size_t _i = 0; _i < block_rows; _i++) {
                for (size_t _j = 0; _j < block_cols; _j++) {
                    result(j + _j, i + _i) = mat(i + _i, j + _j);
                }
            }
        }
    }

    return result;
}

/**
 * @brief Cache oblivious matrix transpose.
 * Inspired by: http://wgropp.cs.illinois.edu/courses/cs598-s16/lectures/lecture08.pdf.
 *
 * @param src Input matrix.
 * @param dst Output matrix.
 * @param row_offset Source matrix row offset.
 * @param rows Number of rows to process.
 * @param col_offset Source matrix column offset.
 * @param cols Number of columns to process.
 */
template <size_t TILE_SIZE = 16, typename T>
void transpose_co(const Matrix<T> &src, CoreMatrix<T> &dst, size_t row_offset, size_t rows,
                  size_t col_offset, size_t cols) {
    if (rows <= TILE_SIZE && cols <= TILE_SIZE) {
        for (size_t i = row_offset; i < rows + row_offset; i++) {
            for (size_t j = col_offset; j < cols + col_offset; j++) {
                dst(j, i) = src(i, j);
            }
        }
    } else if (rows >= cols) {
        // subdivide the long side to keep the submatrices reasonably square
        transpose_co<TILE_SIZE>(src, dst, row_offset, rows / 2, col_offset, cols);
        transpose_co<TILE_SIZE>(src, dst, row_offset + rows / 2, rows - rows / 2, col_offset, cols);
    } else {
        transpose_co<TILE_SIZE>(src, dst, row_offset, rows, col_offset, cols / 2);
        transpose_co<TILE_SIZE>(src, dst, row_offset, rows, col_offset + cols / 2, cols - cols / 2);
    }
}

/**
 * @brief operator +=
 * @param lhs Left hand side, will be modified.
 * @param rhs Right hand side, left untouched.
 * @return Modified lhs holding the result.
 */
template <typename T> CoreMatrix<T> &operator+=(CoreMatrix<T> &lhs, const Matrix<T> &rhs) {
    if (lhs.rows() != rhs.rows() || lhs.cols() != rhs.cols()) {
        SPH_THROW(InvalidArgumentException, "lhs != rhs (cols and rows must be equal)");
    }

    for (size_t i = 0; i < lhs.rows(); i++) {
        for (size_t j = 0; j < lhs.cols(); j++) {
            lhs(i, j) += rhs(i, j);
        }
    }

    return lhs;
}

/**
 * @brief operator +
 * @param lhs Left hand side, left untouched.
 * @param rhs Right hand side, left untouched.
 * @return Newly allocated matrix instance holding the result.
 */
template <typename T> CoreMatrix<T> operator+(const CoreMatrix<T> &lhs, const Matrix<T> &rhs) {
    CoreMatrix result(lhs);

    result += rhs;
    return result;
}

/**
 * @brief operator +=
 * @param lhs Left hand side, will be modified.
 * @param val Value added to each matrix element.
 * @return Modified lhs holding the result.
 */
template <typename MT, typename VT>
// typename std::enable_if_t<!std::is_same_v<MT, VT>>
// typename std::enable_if<!std::is_floating_point<MT>::value, MT>::type
// typename std::enable_if<!std::is_same<MT, VT>::value, MT>::type
typename std::enable_if_t<!std::is_same_v<CoreMatrix<MT>, VT>, CoreMatrix<MT>> &
operator+=(CoreMatrix<MT> &lhs, VT val) {
    for (size_t i = 0; i < lhs.rows(); i++) {
        for (size_t j = 0; j < lhs.cols(); j++) {
            lhs(i, j) = static_cast<MT>(lhs(i, j) + val);
        }
    }

    return lhs;
}

/**
 * @brief operator +
 * @param lhs Left hand side, left untouched.
 * @param val Value added to each matrix element.
 * @return Newly allocated matrix instance holding the result.
 */
template <typename MT, typename VT>
typename std::enable_if_t<!std::is_same_v<CoreMatrix<MT>, VT>, CoreMatrix<MT>>
operator+(const CoreMatrix<MT> &lhs, VT val) {
    CoreMatrix<MT> result(lhs);

    result += val;
    return result;
}

/**
 * @brief operator -=
 * @param lhs Left hand side, will be modified.
 * @param rhs Right hand side, left untouched.
 * @return Modified lhs holding the result.
 */
template <typename T> CoreMatrix<T> &operator-=(CoreMatrix<T> &lhs, const Matrix<T> &rhs) {
    if (lhs.rows() != rhs.rows() || lhs.cols() != rhs.cols()) {
        SPH_THROW(InvalidArgumentException, "lhs != rhs (cols and rows must be equal)");
    }

    for (size_t i = 0; i < lhs.rows(); i++) {
        for (size_t j = 0; j < lhs.cols(); j++) {
            lhs(i, j) -= rhs(i, j);
        }
    }

    return lhs;
}

/**
 * @brief operator -
 * @param lhs Left hand side, left untouched.
 * @param rhs Right hand side, left untouched.
 * @return Newly allocated matrix instance holding the result.
 */
template <typename T> CoreMatrix<T> operator-(const CoreMatrix<T> &lhs, const Matrix<T> &rhs) {
    CoreMatrix result(lhs);

    result -= rhs;
    return result;
}

/**
 * @brief operator -=
 * @param lhs Left hand side, will be modified.
 * @param val Value substracted from each matrix element.
 * @return Modified lhs holding the result.
 */
template <typename MT, typename VT>
typename std::enable_if_t<!std::is_same_v<CoreMatrix<MT>, VT>, CoreMatrix<MT>> &
operator-=(CoreMatrix<MT> &lhs, VT val) {
    for (size_t i = 0; i < lhs.rows(); i++) {
        for (size_t j = 0; j < lhs.cols(); j++) {
            lhs(i, j) = static_cast<MT>(lhs(i, j) - val);
        }
    }

    return lhs;
}

/**
 * @brief operator -
 * @param lhs Left hand side, left untouched.
 * @param val Value substracted from each matrix element.
 * @return Newly allocated matrix instance holding the result.
 */
template <typename MT, typename VT>
typename std::enable_if_t<!std::is_same_v<CoreMatrix<MT>, VT>, CoreMatrix<MT>>
operator-(const CoreMatrix<MT> &lhs, VT val) {
    CoreMatrix<MT> result(lhs);

    result -= val;
    return result;
}

/**
 * @brief operator *=
 * @param lhs Left hand side, will be modified.
 * @param rhs Right hand side, left untouched.
 * @return Modified lhs holding the result.
 */
template <typename T> CoreMatrix<T> &operator*=(CoreMatrix<T> &lhs, const Matrix<T> &rhs) {
    if (lhs.cols() != rhs.rows()) {
        SPH_THROW(InvalidArgumentException, "lhs.cols() != rhs.rows()");
    }

#ifdef WITH_BLAS
    if constexpr (std::is_same_v<T, double>) {
        CoreMatrix<double> _lhs(lhs);
        CoreMatrix<double> _rhs(rhs);
        CoreMatrix<double> result(lhs.rows(), rhs.cols());

        cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, _lhs.rows(), _rhs.cols(),
                    _rhs.rows(), 1.0, _lhs.data(), _lhs.cols(), _rhs.data(), _rhs.cols(), 0.0,
                    result.data(), result.cols());

        lhs = result;
    } else {
        CoreMatrix<float> _lhs(lhs);
        CoreMatrix<float> _rhs(rhs);
        CoreMatrix<float> result(lhs.rows(), rhs.cols());

        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, _lhs.rows(), _rhs.cols(),
                    _rhs.rows(), 1.0, _lhs.data(), _lhs.cols(), _rhs.data(), _rhs.cols(), 0.0,
                    result.data(), result.cols());

        lhs = result;
    }
#else
    CoreMatrix<T> result(lhs.rows(), lhs.cols());

    // avoid cache misses by first transposing rhs
    CoreMatrix rhs_t = transpose(rhs);

    // lhs:     MxN
    // rhs:     NxK
    // result:  MxK
    const size_t M = lhs.rows();
    const size_t N = lhs.cols();
    const size_t K = rhs.cols();
    size_t i, j, l;

    for (i = 0; i < M; i++) {
        for (j = 0; j < K; j++) {
            T tmp = 0;
            for (l = 0; l < N; l++) {
                tmp += lhs(i, l) * rhs_t(j, l);
            }
            result(i, j) = tmp;
        }
    }

    lhs = result;
#endif

    return lhs;
}

/**
 * @brief operator *
 * @param lhs Left hand side, left untouched.
 * @param rhs Right hand side, left untouched.
 * @return Newly allocated matrix instance holding the result.
 */
template <typename T> CoreMatrix<T> operator*(const CoreMatrix<T> &lhs, const Matrix<T> &rhs) {
    CoreMatrix result(lhs);

    result *= rhs;
    return result;
}

/**
 * @brief operator *=
 * @param lhs Left hand side, will be modified.
 * @param val Value multiplied with each matrix element.
 * @return Modified lhs holding the result.
 */
template <typename MT, typename VT>
typename std::enable_if_t<!std::is_same_v<CoreMatrix<MT>, VT>, CoreMatrix<MT>> &
operator*=(CoreMatrix<MT> &lhs, VT val) {
    for (size_t i = 0; i < lhs.rows(); i++) {
        for (size_t j = 0; j < lhs.cols(); j++) {
            lhs(i, j) = static_cast<MT>(lhs(i, j) * val);
        }
    }

    return lhs;
}

/**
 * @brief operator *
 * @param lhs Left hand side, left untouched.
 * @param val Value multiplied with each matrix element.
 * @return Newly allocated matrix instance holding the result.
 */
template <typename MT, typename VT>
typename std::enable_if_t<!std::is_same_v<CoreMatrix<MT>, VT>, CoreMatrix<MT>>
operator*(const CoreMatrix<MT> &lhs, VT val) {
    CoreMatrix<MT> result(lhs);

    result *= val;
    return result;
}

/**
 * @brief operator /=
 * @param lhs Left hand side, will be modified.
 * @param val Value used to divide each matrix element.
 * @return Modified lhs holding the result.
 */
template <typename MT, typename VT>
typename std::enable_if_t<!std::is_same_v<CoreMatrix<MT>, VT>, CoreMatrix<MT>> &
operator/=(CoreMatrix<MT> &lhs, VT val) {
    for (size_t i = 0; i < lhs.rows(); i++) {
        for (size_t j = 0; j < lhs.cols(); j++) {
            lhs(i, j) = static_cast<MT>(lhs(i, j) / val);
        }
    }

    return lhs;
}

/**
 * @brief operator /
 * @param lhs Left hand side, left untouched.
 * @param val Value used to divide each matrix element.
 * @return Newly allocated matrix instance holding the result.
 */
template <typename MT, typename VT>
typename std::enable_if_t<!std::is_same_v<CoreMatrix<MT>, VT>, CoreMatrix<MT>>
operator/(const CoreMatrix<MT> &lhs, VT val) {
    CoreMatrix<MT> result(lhs);

    result /= val;
    return result;
}

} // namespace sph

#endif // SPH_MATRIX_CORE_OPS_HPP
