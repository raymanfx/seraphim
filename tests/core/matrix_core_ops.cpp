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
    SECTION( "transpose_block() returns the transposed matrix" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        CoreMatrix<int> m2 = transpose_block(m1);

        REQUIRE( m2(0, 0) == 1 );
        REQUIRE( m2(0, 1) == 9 );
        REQUIRE( m2(1, 0) == 2 );
        REQUIRE( m2(1, 1) == 8 );

        m1 = CoreMatrix<int>(100, 100);
        m1 = 0;
        m1(0, 0) = 1;
        m1(0, 99) = 2;
        m1(99, 0) = 3;
        m1(99, 99) = 4;

        m2 = transpose_block(m1);

        REQUIRE( m2(0, 0) == 1 );
        REQUIRE( m2(0, 99) == 3 );
        REQUIRE( m2(99, 0) == 2 );
        REQUIRE( m2(99, 99) == 4 );

        m1 = CoreMatrix<int>(50, 29);
        m1 = 0;
        m1(0, 0) = 1;
        m1(0, 28) = 2;
        m1(49, 0) = 3;
        m1(49, 28) = 4;

        m2 = transpose_block(m1);

        REQUIRE( m2(0, 0) == 1 );
        REQUIRE( m2(0, 49) == 3 );
        REQUIRE( m2(28, 0) == 2 );
        REQUIRE( m2(28, 49) == 4 );
    }
    SECTION( "transpose_co() returns the transposed matrix" ) {
        CoreMatrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        CoreMatrix<int> m2(m1.cols(), m1.rows());
        transpose_co(m1, m2, 0, m1.rows(), 0, m1.cols());

        REQUIRE( m2(0, 0) == 1 );
        REQUIRE( m2(0, 1) == 9 );
        REQUIRE( m2(1, 0) == 2 );
        REQUIRE( m2(1, 1) == 8 );

        m1 = CoreMatrix<int>(100, 100);
        m2 = CoreMatrix<int>(100, 100);
        m1 = 0;
        m1(0, 0) = 1;
        m1(0, 99) = 2;
        m1(99, 0) = 3;
        m1(99, 99) = 4;

        transpose_co(m1, m2, 0, m1.rows(), 0, m1.cols());

        REQUIRE( m2(0, 0) == 1 );
        REQUIRE( m2(0, 99) == 3 );
        REQUIRE( m2(99, 0) == 2 );
        REQUIRE( m2(99, 99) == 4 );

        m1 = CoreMatrix<int>(50, 29);
        m2 = CoreMatrix<int>(29, 50);
        m1 = 0;
        m1(0, 0) = 1;
        m1(0, 28) = 2;
        m1(49, 0) = 3;
        m1(49, 28) = 4;

        transpose_co(m1, m2, 0, m1.rows(), 0, m1.cols());

        REQUIRE( m2(0, 0) == 1 );
        REQUIRE( m2(0, 49) == 3 );
        REQUIRE( m2(28, 0) == 2 );
        REQUIRE( m2(28, 49) == 4 );
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
