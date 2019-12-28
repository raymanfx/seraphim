/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <chrono>
#include <vector>

#include <seraphim/matrix.h>

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
    sph::CoreMatrix<uint8_t> mat1;
    sph::CoreMatrix<uint8_t> mat2;
    std::chrono::high_resolution_clock::time_point t0;
    std::chrono::high_resolution_clock::time_point t1;
    long elapsed_ns;
    std::vector<std::pair<size_t, size_t>> sizes = {
        {std::pow(10, 1), std::pow(10, 1)},
        {std::pow(10, 2), std::pow(10, 2)},
        {std::pow(10, 3), std::pow(10, 3)},
        //{std::pow(10, 4), std::pow(10, 4)},
    };
    int runs = 3;

    std::cout << preamble() << std::endl;
    std::cout << ">>> Runs per benchmark: " << runs << std::endl << std::endl;

    std::cout << " * Matrix addition" << std::endl;
    elapsed_ns = 0;
    for (const auto &size : sizes) {
        for (int i = 0; i < runs; i++) {
            mat1 = sph::CoreMatrix<uint8_t>(size.first, size.second);
            mat2 = sph::CoreMatrix<uint8_t>(size.first, size.second);
            mat1 = 1;
            mat2 = 1;
            t0 = std::chrono::high_resolution_clock::now();
            mat1 += mat2;
            t1 = std::chrono::high_resolution_clock::now();

            elapsed_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        }
        std::cout << size.first << "x" << size.second << ": " << timestamp_str(elapsed_ns / runs)
                  << std::endl;
    }

    std::cout << std::endl;
    std::cout << " * Matrix substraction" << std::endl;
    elapsed_ns = 0;
    for (const auto &size : sizes) {
        for (int i = 0; i < runs; i++) {
            mat1 = sph::CoreMatrix<uint8_t>(size.first, size.second);
            mat2 = sph::CoreMatrix<uint8_t>(size.first, size.second);
            mat1 = 1;
            mat2 = 1;
            t0 = std::chrono::high_resolution_clock::now();
            mat1 -= mat2;
            t1 = std::chrono::high_resolution_clock::now();

            elapsed_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        }
        std::cout << size.first << "x" << size.second << ": " << timestamp_str(elapsed_ns / runs)
                  << std::endl;
    }

    std::cout << std::endl;
    std::cout << " * Matrix multiplication" << std::endl;
    elapsed_ns = 0;
    for (const auto &size : sizes) {
        for (int i = 0; i < runs; i++) {
            mat1 = sph::CoreMatrix<uint8_t>(size.first, size.second);
            mat2 = sph::CoreMatrix<uint8_t>(size.first, size.second);
            mat1 = 1;
            mat2 = 1;
            t0 = std::chrono::high_resolution_clock::now();
            mat1 *= mat2;
            t1 = std::chrono::high_resolution_clock::now();

            elapsed_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        }
        std::cout << size.first << "x" << size.second << ": " << timestamp_str(elapsed_ns / runs)
                  << std::endl;
    }

    std::cout << std::endl;
    std::cout << " * Matrix convolution (3x3)" << std::endl;
    elapsed_ns = 0;
    for (const auto &size : sizes) {
        for (int i = 0; i < runs; i++) {
            mat1 = sph::CoreMatrix<uint8_t>(size.first, size.second);
            mat2 = sph::CoreMatrix<uint8_t>(3, 3);
            mat1 = 1;
            mat2 = 1;
            t0 = std::chrono::high_resolution_clock::now();
            sph::convolve(mat1, mat2, sph::EdgeHandling::CLAMP);
            t1 = std::chrono::high_resolution_clock::now();

            elapsed_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        }
        std::cout << size.first << "x" << size.second << ": " << timestamp_str(elapsed_ns / runs)
                  << std::endl;
    }

    std::cout << std::endl;
    std::cout << " * Matrix convolution (5x5)" << std::endl;
    elapsed_ns = 0;
    for (const auto &size : sizes) {
        for (int i = 0; i < runs; i++) {
            mat1 = sph::CoreMatrix<uint8_t>(size.first, size.second);
            mat2 = sph::CoreMatrix<uint8_t>(5, 5);
            mat1 = 1;
            mat2 = 1;
            t0 = std::chrono::high_resolution_clock::now();
            sph::convolve(mat1, mat2, sph::EdgeHandling::CLAMP);
            t1 = std::chrono::high_resolution_clock::now();

            elapsed_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        }
        std::cout << size.first << "x" << size.second << ": " << timestamp_str(elapsed_ns / runs)
                  << std::endl;
    }
}
