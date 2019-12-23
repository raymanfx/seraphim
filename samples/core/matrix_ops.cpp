/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <chrono>
#include <vector>

#include <seraphim/matrix.h>

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
    std::vector<std::pair<size_t, size_t>> sizes = {
        {std::pow(10, 1), std::pow(10, 1)},
        {std::pow(10, 2), std::pow(10, 2)},
        {std::pow(10, 3), std::pow(10, 3)},
        //{std::pow(10, 4), std::pow(10, 4)},
    };

    std::cout << "=== ADDITION ===" << std::endl;
    for (const auto &size : sizes) {
        mat1 = sph::CoreMatrix<uint8_t>(size.first, size.second);
        mat2 = sph::CoreMatrix<uint8_t>(size.first, size.second);
        mat1 = 1;
        mat2 = 1;
        t0 = std::chrono::high_resolution_clock::now();
        mat1 += mat2;
        t1 = std::chrono::high_resolution_clock::now();

        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        std::cout << size.first << "x" << size.second << ": " << timestamp_str(ns) << std::endl;
    }

    std::cout << std::endl;
    std::cout << "=== SUBSTRACTION ===" << std::endl;
    for (const auto &size : sizes) {
        mat1 = sph::CoreMatrix<uint8_t>(size.first, size.second);
        mat2 = sph::CoreMatrix<uint8_t>(size.first, size.second);
        mat1 = 1;
        mat2 = 1;
        t0 = std::chrono::high_resolution_clock::now();
        mat1 -= mat2;
        t1 = std::chrono::high_resolution_clock::now();

        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        std::cout << size.first << "x" << size.second << ": " << timestamp_str(ns) << std::endl;
    }

    std::cout << std::endl;
    std::cout << "=== MULTIPLICATION ===" << std::endl;
    for (const auto &size : sizes) {
        mat1 = sph::CoreMatrix<uint8_t>(size.first, size.second);
        mat2 = sph::CoreMatrix<uint8_t>(size.first, size.second);
        mat1 = 1;
        mat2 = 1;
        t0 = std::chrono::high_resolution_clock::now();
        mat1 *= mat2;
        t1 = std::chrono::high_resolution_clock::now();

        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        std::cout << size.first << "x" << size.second << ": " << timestamp_str(ns) << std::endl;
    }
}
