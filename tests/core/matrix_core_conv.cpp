#include <catch2/catch.hpp>
#include <sstream>

#include <seraphim/matrix.h>

using namespace sph;

TEST_CASE( "Matrix kernel convolution", "[CoreMatrix<T>]" ) {
    SECTION( "edge detection [3x3 ZERO]" ) {
        CoreMatrix<int> m1({
            { 10, 20, 30 },
            { 20, 30, 40 },
            { 30, 40, 50 }
        });

        // edge detection
        CoreMatrix<int> kernel({
            { -1, -1, -1 },
            { -1,  5, -1 },
            { -1, -1, -1 }
        });

        CoreMatrix<int> result = convolve<1>(m1, kernel, EdgeHandling::ZERO);

        REQUIRE( result(0, 0) == -20 );
        REQUIRE( result(0, 1) == -30 );
        REQUIRE( result(0, 2) == 60 );
        REQUIRE( result(1, 0) == -30 );
        REQUIRE( result(1, 1) == -90 );
        REQUIRE( result(1, 2) == 30 );
        REQUIRE( result(2, 0) == 60 );
        REQUIRE( result(2, 1) == 30 );
        REQUIRE( result(2, 2) == 140 );
    }
    SECTION( "edge detection [3x3 CLAMP]" ) {
        CoreMatrix<int> m1({
            { 10, 20, 30 },
            { 20, 30, 40 },
            { 30, 40, 50 }
        });

        // edge detection
        CoreMatrix<int> kernel({
            { -1, -1, -1 },
            { -1,  5, -1 },
            { -1, -1, -1 }
        });

        CoreMatrix<int> result = convolve<1>(m1, kernel, EdgeHandling::CLAMP);

        REQUIRE( result(0, 0) == -90 );
        REQUIRE( result(0, 1) == -90 );
        REQUIRE( result(0, 2) == -90 );
        REQUIRE( result(1, 0) == -90 );
        REQUIRE( result(1, 1) == -90 );
        REQUIRE( result(1, 2) == -90 );
        REQUIRE( result(2, 0) == -90 );
        REQUIRE( result(2, 1) == -90 );
        REQUIRE( result(2, 2) == -90 );
    }
    SECTION( "gaussian blur [5x5 ZERO]" ) {
        CoreMatrix<int> m1({
            { 10, 10, 10 },
            { 10, 10, 10 },
            { 10, 10, 10 }
        });

        // gaussian blur
        CoreMatrix<int> kernel({
            { 1, 1, 1, 1, 1 },
            { 1, 2, 2, 2, 1 },
            { 1, 2, 4, 2, 1 },
            { 1, 2, 2, 2, 1 },
            { 1, 1, 1, 1, 1 }
        });

        CoreMatrix<int> result = convolve<1>(m1, kernel, EdgeHandling::ZERO);

        REQUIRE( result(0, 0) == 150 );
        REQUIRE( result(0, 1) == 170 );
        REQUIRE( result(0, 2) == 150 );
        REQUIRE( result(1, 0) == 170 );
        REQUIRE( result(1, 1) == 200 );
        REQUIRE( result(1, 2) == 170 );
        REQUIRE( result(2, 0) == 150 );
        REQUIRE( result(2, 1) == 170 );
        REQUIRE( result(2, 2) == 150 );
    }
    SECTION( "gaussian blur [5x5 CLAMP]" ) {
        CoreMatrix<int> m1({
            { 10, 10, 10 },
            { 10, 10, 10 },
            { 10, 10, 10 }
        });

        // gaussian blur
        CoreMatrix<int> kernel({
            { 1, 1, 1, 1, 1 },
            { 1, 2, 2, 2, 1 },
            { 1, 2, 4, 2, 1 },
            { 1, 2, 2, 2, 1 },
            { 1, 1, 1, 1, 1 }
        });

        CoreMatrix<int> result = convolve<1>(m1, kernel, EdgeHandling::CLAMP);

        REQUIRE( result(0, 0) == 360 );
        REQUIRE( result(0, 1) == 360 );
        REQUIRE( result(0, 2) == 360 );
        REQUIRE( result(1, 0) == 360 );
        REQUIRE( result(1, 1) == 360 );
        REQUIRE( result(1, 2) == 360 );
        REQUIRE( result(2, 0) == 360 );
        REQUIRE( result(2, 1) == 360 );
        REQUIRE( result(2, 2) == 360 );
    }
    SECTION( "Grayscale convolution [3x3 ZERO]" ) {
        CoreMatrix<int> m1(3, 3 * 3);
        m1 = 1;

        // edge detection
        CoreMatrix<int> kernel({
            { -1, -1, -1 },
            { -1,  5, -1 },
            { -1, -1, -1 }
        });

        CoreMatrix<int> result = convolve<1>(m1, kernel, EdgeHandling::ZERO);

        REQUIRE( result(0, 0) == 2 );
        REQUIRE( result(0, 1) == 0 );
        REQUIRE( result(0, 2) == 0 );
        REQUIRE( result(0, 3) == 0 );
        REQUIRE( result(0, 4) == 0 );
        REQUIRE( result(0, 5) == 0 );
        REQUIRE( result(0, 6) == 0 );
        REQUIRE( result(0, 7) == 0 );
        REQUIRE( result(0, 8) == 2 );
        REQUIRE( result(1, 0) == 0 );
        REQUIRE( result(1, 1) == -3 );
        REQUIRE( result(1, 2) == -3 );
        REQUIRE( result(1, 3) == -3 );
        REQUIRE( result(1, 4) == -3 );
        REQUIRE( result(1, 5) == -3 );
        REQUIRE( result(1, 6) == -3 );
        REQUIRE( result(1, 7) == -3 );
        REQUIRE( result(1, 8) == 0 );
        REQUIRE( result(2, 0) == 2 );
        REQUIRE( result(2, 1) == 0 );
        REQUIRE( result(2, 2) == 0 );
        REQUIRE( result(2, 3) == 0 );
        REQUIRE( result(2, 4) == 0 );
        REQUIRE( result(2, 5) == 0 );
        REQUIRE( result(2, 6) == 0 );
        REQUIRE( result(2, 7) == 0 );
        REQUIRE( result(2, 8) == 2 );
    }
    SECTION( "RGB convolution [3x3 ZERO]" ) {
        CoreMatrix<int> m1(3, 3 * 3);
        m1 = 1;

        // edge detection
        CoreMatrix<int> kernel({
            { -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { -1, -1, -1,  5,  5,  5, -1, -1, -1 },
            { -1, -1, -1, -1, -1, -1, -1, -1, -1 }
        });

        CoreMatrix<int> result = convolve<3, 3>(m1, kernel, EdgeHandling::ZERO);

        REQUIRE( result(0, 0) == 2 );
        REQUIRE( result(0, 1) == 2 );
        REQUIRE( result(0, 2) == 2 );
        REQUIRE( result(0, 3) == 0 );
        REQUIRE( result(0, 4) == 0 );
        REQUIRE( result(0, 5) == 0 );
        REQUIRE( result(0, 6) == 2 );
        REQUIRE( result(0, 7) == 2 );
        REQUIRE( result(0, 8) == 2 );
        REQUIRE( result(1, 0) == 0 );
        REQUIRE( result(1, 1) == 0 );
        REQUIRE( result(1, 2) == 0 );
        REQUIRE( result(1, 3) == -3 );
        REQUIRE( result(1, 4) == -3 );
        REQUIRE( result(1, 5) == -3 );
        REQUIRE( result(1, 6) == 0 );
        REQUIRE( result(1, 7) == 0 );
        REQUIRE( result(1, 8) == 0 );
        REQUIRE( result(2, 0) == 2 );
        REQUIRE( result(2, 1) == 2 );
        REQUIRE( result(2, 2) == 2 );
        REQUIRE( result(2, 3) == 0 );
        REQUIRE( result(2, 4) == 0 );
        REQUIRE( result(2, 5) == 0 );
        REQUIRE( result(2, 6) == 2 );
        REQUIRE( result(2, 7) == 2 );
        REQUIRE( result(2, 8) == 2 );
    }
    SECTION( "RGB convolution [3x3 ZERO], channel scaling" ) {
        CoreMatrix<int> m1(3, 3 * 3);
        m1 = 1;

        // edge detection
        CoreMatrix<int> kernel({
            { -1, -1, -1 },
            { -1,  5, -1 },
            { -1, -1, -1 }
        });

        CoreMatrix<int> result = convolve<3, 1>(m1, kernel, EdgeHandling::ZERO);

        REQUIRE( result(0, 0) == 2 );
        REQUIRE( result(0, 1) == 2 );
        REQUIRE( result(0, 2) == 2 );
        REQUIRE( result(0, 3) == 0 );
        REQUIRE( result(0, 4) == 0 );
        REQUIRE( result(0, 5) == 0 );
        REQUIRE( result(0, 6) == 2 );
        REQUIRE( result(0, 7) == 2 );
        REQUIRE( result(0, 8) == 2 );
        REQUIRE( result(1, 0) == 0 );
        REQUIRE( result(1, 1) == 0 );
        REQUIRE( result(1, 2) == 0 );
        REQUIRE( result(1, 3) == -3 );
        REQUIRE( result(1, 4) == -3 );
        REQUIRE( result(1, 5) == -3 );
        REQUIRE( result(1, 6) == 0 );
        REQUIRE( result(1, 7) == 0 );
        REQUIRE( result(1, 8) == 0 );
        REQUIRE( result(2, 0) == 2 );
        REQUIRE( result(2, 1) == 2 );
        REQUIRE( result(2, 2) == 2 );
        REQUIRE( result(2, 3) == 0 );
        REQUIRE( result(2, 4) == 0 );
        REQUIRE( result(2, 5) == 0 );
        REQUIRE( result(2, 6) == 2 );
        REQUIRE( result(2, 7) == 2 );
        REQUIRE( result(2, 8) == 2 );
    }
}
