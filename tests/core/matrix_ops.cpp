#include <catch2/catch.hpp>
#include <sstream>

#include <seraphim/matrix.h>

using namespace sph;

TEST_CASE( "Matrix operators", "[CoreMatrix<T>]" ) {
    SECTION( "transpose() returns the transposed matrix" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        CoreMatrix<int> m2 = transpose(m1);

        REQUIRE( m2(0, 0) == 1 );
        REQUIRE( m2(0, 1) == 9 );
        REQUIRE( m2(1, 0) == 2 );
        REQUIRE( m2(1, 1) == 8 );
    }
    SECTION( "+= operator performs element addition" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        CoreMatrix<int> m2({
            { 1, 2 },
            { 3, 4 }
        });
        m1 += m2;

        REQUIRE( m1(0, 0) == 2 );
        REQUIRE( m1(0, 1) == 4 );
        REQUIRE( m1(1, 0) == 12 );
        REQUIRE( m1(1, 1) == 12 );
    }
    SECTION( "+ operator performs element addition" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        CoreMatrix<int> m2({
            { 1, 2 },
            { 3, 4 }
        });
        CoreMatrix<int> m3 = m1 + m2;

        REQUIRE( m3(0, 0) == 2 );
        REQUIRE( m3(0, 1) == 4 );
        REQUIRE( m3(1, 0) == 12 );
        REQUIRE( m3(1, 1) == 12 );
    }
    SECTION( "+= operator adds a scalar value to each element" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        m1 += 2;

        REQUIRE( m1(0, 0) == 3 );
        REQUIRE( m1(0, 1) == 4 );
        REQUIRE( m1(1, 0) == 11 );
        REQUIRE( m1(1, 1) == 10 );
    }
    SECTION( "+ operator adds a scalar value to each element" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        CoreMatrix<int> m3 = m1 + 2;

        REQUIRE( m3(0, 0) == 3 );
        REQUIRE( m3(0, 1) == 4 );
        REQUIRE( m3(1, 0) == 11 );
        REQUIRE( m3(1, 1) == 10 );
    }
    SECTION( "-= operator performs element substraction" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        CoreMatrix<int> m2({
            { 1, 2 },
            { 3, 4 }
        });
        m1 -= m2;

        REQUIRE( m1(0, 0) == 0 );
        REQUIRE( m1(0, 1) == 0 );
        REQUIRE( m1(1, 0) == 6 );
        REQUIRE( m1(1, 1) == 4 );
    }
    SECTION( "- operator performs element substraction" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        CoreMatrix<int> m2({
            { 1, 2 },
            { 3, 4 }
        });
        CoreMatrix<int> m3 = m1 - m2;

        REQUIRE( m3(0, 0) == 0 );
        REQUIRE( m3(0, 1) == 0 );
        REQUIRE( m3(1, 0) == 6 );
        REQUIRE( m3(1, 1) == 4 );
    }
    SECTION( "-= operator substracts a scalar value from each element" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        m1 -= 2;

        REQUIRE( m1(0, 0) == -1 );
        REQUIRE( m1(0, 1) == 0 );
        REQUIRE( m1(1, 0) == 7 );
        REQUIRE( m1(1, 1) == 6 );
    }
    SECTION( "- operator substracts a scalar value from each element" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        CoreMatrix<int> m3 = m1 - 2;

        REQUIRE( m3(0, 0) == -1 );
        REQUIRE( m3(0, 1) == 0 );
        REQUIRE( m3(1, 0) == 7 );
        REQUIRE( m3(1, 1) == 6 );
    }
    SECTION( "*= operator performs matrix multiplication" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        CoreMatrix<int> m2({
            { 1, 2 },
            { 3, 4 }
        });
        m1 *= m2;

        REQUIRE( m1(0, 0) == 7 );
        REQUIRE( m1(0, 1) == 10 );
        REQUIRE( m1(1, 0) == 33 );
        REQUIRE( m1(1, 1) == 50 );
    }
    SECTION( "* operator performs matrix multiplication" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        CoreMatrix<int> m2({
            { 1, 2 },
            { 3, 4 }
        });
        CoreMatrix<int> m3 = m1 * m2;

        REQUIRE( m3(0, 0) == 7 );
        REQUIRE( m3(0, 1) == 10 );
        REQUIRE( m3(1, 0) == 33 );
        REQUIRE( m3(1, 1) == 50 );
    }
    SECTION( "*= operator multiplies each element with a scalar value" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        m1 *= 2;

        REQUIRE( m1(0, 0) == 2 );
        REQUIRE( m1(0, 1) == 4 );
        REQUIRE( m1(1, 0) == 18 );
        REQUIRE( m1(1, 1) == 16 );
    }
    SECTION( "* operator multiplies each element with a scalar value" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        CoreMatrix<int> m3 = m1 * 2;

        REQUIRE( m3(0, 0) == 2 );
        REQUIRE( m3(0, 1) == 4 );
        REQUIRE( m3(1, 0) == 18 );
        REQUIRE( m3(1, 1) == 16 );
    }
    SECTION( "/= operator divides each element by a scalar value" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        m1 /= 2;

        REQUIRE( m1(0, 0) == 0 );
        REQUIRE( m1(0, 1) == 1 );
        REQUIRE( m1(1, 0) == 4 );
        REQUIRE( m1(1, 1) == 4 );
    }
    SECTION( "/ operator divides each element by a scalar value" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        CoreMatrix<int> m3 = m1 / 2;

        REQUIRE( m3(0, 0) == 0 );
        REQUIRE( m3(0, 1) == 1 );
        REQUIRE( m3(1, 0) == 4 );
        REQUIRE( m3(1, 1) == 4 );
    }
    SECTION( "ostream operator gives a human readable representation of the elements" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        std::stringstream m1_strstr;
        std::string m1_str;
        m1_strstr << m1;
        m1_str = m1_strstr.str();

        REQUIRE( m1_str[0] == '\n' );
        REQUIRE( m1_str[1] == '[' );
        REQUIRE( m1_str[2] == '[' );
        REQUIRE( m1_str[3] == '1' );
        REQUIRE( m1_str[4] == ' ' );
        REQUIRE( m1_str[5] == '2' );
        REQUIRE( m1_str[6] == ']' );
        REQUIRE( m1_str[7] == '\n' );
        REQUIRE( m1_str[8] == ' ' );
        REQUIRE( m1_str[9] == '[' );
        REQUIRE( m1_str[10] == '9' );
        REQUIRE( m1_str[11] == ' ' );
        REQUIRE( m1_str[12] == '8' );
        REQUIRE( m1_str[13] == ']' );
        REQUIRE( m1_str[14] == ']' );
    }
}

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

        CoreMatrix<int> result = convolve(m1, kernel, EdgeHandling::ZERO);

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

        CoreMatrix<int> result = convolve(m1, kernel, EdgeHandling::CLAMP);

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
    SECTION( "edge detection [5x5 ZERO]" ) {
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

        CoreMatrix<int> result = convolve(m1, kernel, EdgeHandling::ZERO);

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
    SECTION( "edge detection [5x5 CLAMP]" ) {
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

        CoreMatrix<int> result = convolve(m1, kernel, EdgeHandling::CLAMP);

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
}
