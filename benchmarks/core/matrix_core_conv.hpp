#ifndef BENCH_MATRIX_CORE_CONV_HPP
#define BENCH_MATRIX_CORE_CONV_HPP

#include <chrono>
#include <seraphim/matrix.h>

template <typename T, size_t channels>
std::chrono::nanoseconds matrix_convolve(size_t matrix_size, size_t kernel_size) {
    std::chrono::high_resolution_clock::time_point t0;
    std::chrono::high_resolution_clock::time_point t1;
    sph::CoreMatrix<T> matrix(matrix_size, matrix_size * channels);
    sph::CoreMatrix<T> kernel(kernel_size, kernel_size);

    t0 = std::chrono::high_resolution_clock::now();

    sph::convolve<channels>(matrix, kernel, sph::EdgeHandling::CLAMP);

    t1 = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0);
}

#endif // BENCH_MATRIX_CORE_CONV_HPP
