/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "matrix_core_conv.hpp"
#include "matrix_core_ops.hpp"

std::string preamble() {
    std::string ret;
    std::string v_major(SPH_VERSION_MAJOR);
    std::string v_minor(SPH_VERSION_MINOR);
    std::string v_patch(SPH_VERSION_PATCH);
    std::string timestamp(SPH_TIMESTAMP);

    ret += "=\n";
    ret += "| Seraphim " + v_major + "." + v_minor + "." + v_patch + "\n";
    ret += "| Build date: " + timestamp + "\n";
    ret += "=";

    return ret;
}

std::string timestamp_str(long ns) {
    std::string str;
    int exp = 9; // nanoseconds

    while (ns > 1000 && exp > 3) {
        ns /= 1000;
        exp -= 3;
    }
    switch (exp) {
    case 1:
        str = std::to_string(ns) + " s";
        break;
    case 3:
        str = std::to_string(ns) + " ms";
        break;
    case 6:
        str = std::to_string(ns) + " us";
        break;
    case 9:
        str = std::to_string(ns) + " ns";
        break;
    default:
        str = std::to_string(ns) + " ?";
        break;
    }

    return str;
}

int main(int, char **) {
    long elapsed_ns;
    std::vector<size_t> sizes = {
        static_cast<size_t>(std::pow(10, 1)),
        static_cast<size_t>(std::pow(10, 2)),
        static_cast<size_t>(std::pow(10, 3))
    };
    int runs = 3;

    std::cout << preamble() << std::endl;
    std::cout << ">>> Runs per benchmark: " << runs << std::endl << std::endl;

    std::cout << " * Matrix transpose" << std::endl;
    elapsed_ns = 0;
    for (const auto &size : sizes) {
        for (int i = 0; i < runs; i++) {
            elapsed_ns += matrix_transpose<int>(size).count();
        }
        std::cout << size << "x" << size << ": " << timestamp_str(elapsed_ns / runs) << std::endl;
    }

    std::cout << std::endl;
    std::cout << " * Matrix addition" << std::endl;
    elapsed_ns = 0;
    for (const auto &size : sizes) {
        for (int i = 0; i < runs; i++) {
            elapsed_ns += matrix_add<int>(size).count();
        }
        std::cout << size << "x" << size << ": " << timestamp_str(elapsed_ns / runs) << std::endl;
    }

    std::cout << std::endl;
    std::cout << " * Matrix substraction" << std::endl;
    elapsed_ns = 0;
    for (const auto &size : sizes) {
        for (int i = 0; i < runs; i++) {
            elapsed_ns += matrix_substract<int>(size).count();
        }
        std::cout << size << "x" << size << ": " << timestamp_str(elapsed_ns / runs) << std::endl;
    }

    std::cout << std::endl;
    std::cout << " * Matrix multiplication" << std::endl;
    elapsed_ns = 0;
    for (const auto &size : sizes) {
        for (int i = 0; i < runs; i++) {
            elapsed_ns += matrix_multiply<int>(size).count();
        }
        std::cout << size << "x" << size << ": " << timestamp_str(elapsed_ns / runs) << std::endl;
    }

    std::cout << std::endl;
    std::cout << " * Matrix convolution (3x3, one channel)" << std::endl;
    elapsed_ns = 0;
    for (const auto &size : sizes) {
        for (int i = 0; i < runs; i++) {
            elapsed_ns += matrix_convolve<int, 1>(size, 3).count();
        }
        std::cout << size << "x" << size << ": " << timestamp_str(elapsed_ns / runs) << std::endl;
    }

    std::cout << std::endl;
    std::cout << " * Matrix convolution (3x3, three channels)" << std::endl;
    elapsed_ns = 0;
    for (const auto &size : sizes) {
        for (int i = 0; i < runs; i++) {
            elapsed_ns += matrix_convolve<int, 3>(size, 3).count();
        }
        std::cout << size << "x" << size * 3 << ": " << timestamp_str(elapsed_ns / runs)
                  << std::endl;
    }

    std::cout << std::endl;
    std::cout << " * Matrix convolution (5x5, one channel)" << std::endl;
    elapsed_ns = 0;
    for (const auto &size : sizes) {
        for (int i = 0; i < runs; i++) {
            elapsed_ns += matrix_convolve<int, 1>(size, 5).count();
        }
        std::cout << size << "x" << size << ": " << timestamp_str(elapsed_ns / runs) << std::endl;
    }

    std::cout << std::endl;
    std::cout << " * Matrix convolution (5x5, three channels)" << std::endl;
    elapsed_ns = 0;
    for (const auto &size : sizes) {
        for (int i = 0; i < runs; i++) {
            elapsed_ns += matrix_convolve<int, 3>(size, 5).count();
        }
        std::cout << size << "x" << size * 3 << ": " << timestamp_str(elapsed_ns / runs)
                  << std::endl;
    }
}
