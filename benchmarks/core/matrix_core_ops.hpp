#ifndef BENCH_MATRIX_CORE_OPS_HPP
#define BENCH_MATRIX_CORE_OPS_HPP

#include <chrono>
#include <seraphim/matrix.hpp>

template <typename T>
std::chrono::nanoseconds matrix_transpose(size_t size) {
    std::chrono::high_resolution_clock::time_point t0;
    std::chrono::high_resolution_clock::time_point t1;
    sph::CoreMatrix<T> m1(size, size);

    t0 = std::chrono::high_resolution_clock::now();

    sph::transpose(m1);

    t1 = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0);
}

template <typename T>
std::chrono::nanoseconds matrix_add(size_t size) {
    std::chrono::high_resolution_clock::time_point t0;
    std::chrono::high_resolution_clock::time_point t1;
    sph::CoreMatrix<T> m1(size, size);
    sph::CoreMatrix<T> m2(size, size);

    t0 = std::chrono::high_resolution_clock::now();

    m1 += m2;

    t1 = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0);
}

template <typename T>
std::chrono::nanoseconds matrix_substract(size_t size) {
    std::chrono::high_resolution_clock::time_point t0;
    std::chrono::high_resolution_clock::time_point t1;
    sph::CoreMatrix<T> m1(size, size);
    sph::CoreMatrix<T> m2(size, size);

    t0 = std::chrono::high_resolution_clock::now();

    m1 -= m2;

    t1 = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0);
}

template <typename T>
std::chrono::nanoseconds matrix_multiply(size_t size) {
    std::chrono::high_resolution_clock::time_point t0;
    std::chrono::high_resolution_clock::time_point t1;
    sph::CoreMatrix<T> m1(size, size);
    sph::CoreMatrix<T> m2(size, size);

    t0 = std::chrono::high_resolution_clock::now();

    m1 *= m2;

    t1 = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0);
}

#endif // BENCH_MATRIX_CORE_OPS_HPP
