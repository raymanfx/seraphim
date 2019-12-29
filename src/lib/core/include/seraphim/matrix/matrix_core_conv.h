/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_MATRIX_CORE_CONV_H
#define SPH_MATRIX_CORE_CONV_H

#include <array>

#include "seraphim/except.h"

namespace sph {

template <typename T> class Matrix;
template <typename T> class CoreMatrix;

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
CoreMatrix<MT> convolve_1(const Matrix<MT> &mat, const Matrix<KT> &kernel,
                          EdgeHandling mode = EdgeHandling::CLAMP) {
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
            out = { 0 };

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
 * @brief Perform kernel convolution.
 * @param mat Input matrix.
 * @param kernel Convolution kernel, must be square.
 * @return Output matrix.
 */
template <size_t kernel_channels, typename MT, typename KT>
CoreMatrix<MT> convolve_3(const Matrix<MT> &mat, const Matrix<KT> &kernel,
                          EdgeHandling mode = EdgeHandling::CLAMP) {
    CoreMatrix<MT> result(mat.rows(), mat.cols());
    std::array<MT, 3> in;
    std::array<MT, 3> out;
    constexpr auto input_channels = 3;

    // if we need to scale the kernel (i.e. because there are 3 input channels, but only 1 kernel
    // channel), the kernel offsets have to be adjusted
    constexpr auto k_off_1 = kernel_channels > 1 ? 1 : 0;
    constexpr auto k_off_2 = kernel_channels > 1 ? 2 : 0;
    constexpr auto k_scale = kernel_channels > 1 ? 1 : 3;

    // check precondition: same number of channels per kernel element
    if (kernel.cols() % kernel_channels != 0) {
        SPH_THROW(InvalidArgumentException, "each kernel column must be present for all channels");
    }

    // check precondition: square kernel
    if (kernel.rows() != kernel.cols() / kernel_channels) {
        SPH_THROW(InvalidArgumentException, "kernel.rows() != kernel.cols()");
    }

    // check precondition: anchor is kernel center element
    if (kernel.rows() % 2 == 0) {
        SPH_THROW(InvalidArgumentException, "kernel size must be uneven");
    }

    // iterate through all input elements
    for (size_t i = 0; i < mat.rows(); i++) {
        for (size_t j = 0; j < mat.cols(); j += input_channels) {
            out = { 0 };

            // for each input element, apply the kernel convolution
            for (size_t k_i = 0; k_i < kernel.rows(); k_i++) {
                for (size_t k_j = 0; k_j < kernel.cols(); k_j += kernel_channels) {
                    ssize_t in_row = i - kernel.rows() / 2 + k_i;
                    ssize_t in_col =
                        j - kernel.cols() / kernel_channels / 2 * kernel_channels * k_scale +
                        k_j * k_scale;

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
                                in_col = mat.cols() - input_channels;
                            }
                            break;
                        }
                    }

                    // load the input element from the input matrix
                    in[0] = mat(in_row, in_col);
                    in[1] = mat(in_row, in_col + 1);
                    in[2] = mat(in_row, in_col + 2);

                    // accumulate computed values
                    out[0] += static_cast<MT>(in[0] * kernel(k_i, k_j));
                    out[1] += static_cast<MT>(in[1] * kernel(k_i, k_j + k_off_1));
                    out[2] += static_cast<MT>(in[2] * kernel(k_i, k_j + k_off_2));
                }
            }

            result(i, j) = out[0];
            result(i, j + 1) = out[1];
            result(i, j + 2) = out[2];
        }
    }

    return result;
}

/**
 * @brief Perform kernel convolution.
 * @param mat Input matrix.
 * @param kernel Convolution kernel, must be square.
 * @return Output matrix.
 */
template <size_t input_channels = 1, size_t kernel_channels = 1, typename MT, typename KT>
CoreMatrix<MT> convolve(const Matrix<MT> &mat, const Matrix<KT> &kernel,
                        EdgeHandling mode = EdgeHandling::CLAMP) {
    if constexpr (input_channels == 1) {
        return convolve_1(mat, kernel, mode);
    } else if constexpr (input_channels == 3) {
        return convolve_3<kernel_channels>(mat, kernel, mode);
    }

    SPH_THROW(InvalidArgumentException, "invalid number of channels (must be 1 or 3)");
}

} // namespace sph

#endif // SPH_MATRIX_CORE_CONV_H
