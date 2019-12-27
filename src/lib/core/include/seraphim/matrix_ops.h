/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_MATRIX_OPS_H
#define SPH_CORE_MATRIX_OPS_H

#include <iostream>

#ifdef WITH_BLAS
#include <openblas/cblas.h>
#endif

namespace sph {

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

enum class EdgeHandling {
    /// Padding mode, fill 'missing' input values with zeros
    ZERO,
    /// Extension mode, the nearest border values are replicated
    CLAMP
};

/**
 * @brief Perform kernel convolution.
 * @param mat Input matrix.
 * @param kernel Convolution kernel, must be square.
 * @return Output matrix.
 */
template <typename MT, typename KT>
CoreMatrix<MT> convolve(const Matrix<MT> &mat, const Matrix<KT> &kernel, EdgeHandling mode) {
    CoreMatrix<MT> result(mat.rows(), mat.cols());
    MT in;
    MT out;

    // check precondition: square kernel
    if (kernel.rows() != kernel.cols()) {
        SPH_THROW(InvalidArgumentException, "kernel.rows() != kernel.cols()");
    }

    // check precondition: anchor is kernel center element
    if (kernel.rows() % 2 == 0) {
        SPH_THROW(InvalidArgumentException, "kernel size must be uneven");
    }

    // iterate through all input elements
    for (size_t i = 0; i < mat.rows(); i++) {
        for (size_t j = 0; j < mat.cols(); j++) {
            out = 0;

            // for each input element, apply the kernel convolution
            for (size_t k_i = 0; k_i < kernel.rows(); k_i++) {
                for (size_t k_j = 0; k_j < kernel.cols(); k_j++) {
                    ssize_t in_row = i - kernel.rows() / 2 + k_i;
                    ssize_t in_col = j - kernel.cols() / 2 + k_j;

                    // perform edge handling in-place instead of recreating the input matrix
                    if ((in_row < 0 || in_row > mat.rows() - 1) ||
                        (in_col < 0 || in_col > mat.cols() - 1)) {
                        switch (mode) {
                        case EdgeHandling::ZERO:
                            continue;
                        case EdgeHandling::CLAMP:
                            if (in_row < 0) {
                                in_row = 0;
                            } else if (in_row >= mat.rows()) {
                                in_row = mat.rows() - 1;
                            }
                            if (in_col < 0) {
                                in_col = 0;
                            } else if (in_col >= mat.cols()) {
                                in_col = mat.cols() - 1;
                            }
                            break;
                        }
                    }

                    // load the input element from the input matrix
                    in = mat(in_row, in_col);

                    // accumulate computed values
                    out += static_cast<MT>(in * kernel(k_i, k_j));
                }
            }

            result(i, j) = out;
        }
    }

    return result;
}

/**
 * @brief Ostream operator.
 *        Prints the matrix elements organized as rows.
 * @param os Source ostream.
 * @param m Matrix instance.
 * @return Modified ostream.
 */
template <typename T> std::ostream &operator<<(std::ostream &os, CoreMatrix<T> &m) {
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

#endif // SPH_CORE_MATRIX_OPS_H
