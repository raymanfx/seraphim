#include <catch2/catch.hpp>
#include <sstream>

#include <seraphim/core/matrix.h>

using namespace sph::core;

TEST_CASE( "Matrix constructor", "[Matrix<T>]" ) {
    SECTION( "default constructor creates empty instances" ) {
        Matrix<int> m1;

        REQUIRE( m1.empty() );
    }
    SECTION( "new matrices can be allocated" ) {
        Matrix<int> m1(20, 40);

        REQUIRE( !m1.empty() );
        REQUIRE( m1.rows() == 20 );
        REQUIRE( m1.cols() == 40 );
    }
    SECTION( "elements can be assigned from arbitrary sources (zero-copy)" ) {
        float elements[] = { 2.0f, 3.0f, 9.0f, 7.0f };
        Matrix<float> m1(elements, 1, 4);
        Matrix<float> m2(elements, 2, 2);

        REQUIRE( m1[0][0] == 2.0f );
        REQUIRE( m1[0][1] == 3.0f );
        REQUIRE( m1[0][2] == 9.0f );
        REQUIRE( m1[0][3] == 7.0f );
        REQUIRE( m2[0][0] == 2.0f );
        REQUIRE( m2[0][1] == 3.0f );
        REQUIRE( m2[1][0] == 9.0f );
        REQUIRE( m2[1][1] == 7.0f );
    }
    SECTION( "elements can be copied from 2d arrays allocated on the stack" ) {
        double elements[][3] = {
            { 1.0, 3.0, 5.0 },
            { 2.0, 4.0, 6.0 }
        };
        Matrix<double> m1(elements);

        REQUIRE( m1[0][0] == 1.0 );
        REQUIRE( m1[0][1] == 3.0 );
        REQUIRE( m1[0][2] == 5.0 );
        REQUIRE( m1[1][0] == 2.0 );
        REQUIRE( m1[1][1] == 4.0 );
        REQUIRE( m1[1][2] == 6.0 );
    }
    SECTION( "elements can be copied from 2d vectors" ) {
        std::vector<std::vector<int>> elements = {
            { 1, 2 },
            { 9, 8 },
            { 1, 3 }
        };
        Matrix<int> m1(elements);

        REQUIRE( m1[0][0] == 1 );
        REQUIRE( m1[0][1] == 2 );
        REQUIRE( m1[1][0] == 9 );
        REQUIRE( m1[1][1] == 8 );
        REQUIRE( m1[2][0] == 1 );
        REQUIRE( m1[2][1] == 3 );
    }
    SECTION( "copy constructor performs a deep copy of the source matrix" ) {
        Matrix<int> m1({
            { 1, 2 },
            { 9, 8 },
            { 1, 3 }
        });
        Matrix<int> m2(m1);

        REQUIRE( m2.data() != m1.data() );
        REQUIRE( m2.rows() == m1.rows() );
        REQUIRE( m2.cols() == m1.cols() );
        REQUIRE( m2[0][0] == m1[0][0] );
        REQUIRE( m2[0][1] == m1[0][1] );
        REQUIRE( m2[1][0] == m1[1][0] );
        REQUIRE( m2[1][1] == m1[1][1] );
        REQUIRE( m2[2][0] == m1[2][0] );
        REQUIRE( m2[2][1] == m1[2][1] );
    }
}

TEST_CASE( "Matrix runtime behavior", "[Matrix<T>]" ) {
    SECTION( "copy assignment operator performs a shallow copy" ) {
        Matrix<int> m1({
            { 1, 2 },
            { 9, 8 },
            { 1, 3 }
        });
        Matrix<int> m2;
        m2 = m1;

        REQUIRE( m2.data() == m1.data() );
        REQUIRE( m2.rows() == m1.rows() );
        REQUIRE( m2.cols() == m1.cols() );
        REQUIRE( m2[0][0] == m1[0][0] );
        REQUIRE( m2[0][1] == m1[0][1] );
        REQUIRE( m2[1][0] == m1[1][0] );
        REQUIRE( m2[1][1] == m1[1][1] );
        REQUIRE( m2[2][0] == m1[2][0] );
        REQUIRE( m2[2][1] == m1[2][1] );
    }
    SECTION( "array subscript operator returns correct elements" ) {
        Matrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });

        REQUIRE( m1[0][0] == 1 );
        REQUIRE( m1[0][1] == 2 );
        REQUIRE( m1[1][0] == 9 );
        REQUIRE( m1[1][1] == 8 );
    }
    SECTION( "array subscript operator can alter elements" ) {
        Matrix<int> m1({
            { 1, 2 },
            { 9, 8 }
        });
        m1[0][1] += 1;
        m1[1][0] *= 3;

        REQUIRE( m1[0][0] == 1 );
        REQUIRE( m1[0][1] == 3 );
        REQUIRE( m1[1][0] == 27 );
        REQUIRE( m1[1][1] == 8 );
    }
    SECTION( "subscript operator extracts valid regions" ) {
        Matrix<int> m1({
            { 1, 2 },
            { 9, 8 },
            { 3, 6 },
            { 7, 8 }
        });
        Matrix<int> m1_top = m1(0, 0, 2, 2);
        Matrix<int> m1_left = m1(0, 0, 4, 1);
        Matrix<int> m1_right = m1(0, 1, 4, 1);
        Matrix<int> m1_bottom = m1(2, 0, 2, 2);

        REQUIRE( m1_top[0][0] == 1 );
        REQUIRE( m1_top[0][1] == 2 );
        REQUIRE( m1_top[1][0] == 9 );
        REQUIRE( m1_top[1][1] == 8 );
        REQUIRE( m1_left[0][0] == 1 );
        REQUIRE( m1_left[1][0] == 9 );
        REQUIRE( m1_left[2][0] == 3 );
        REQUIRE( m1_left[3][0] == 7 );
        REQUIRE( m1_right[0][0] == 2 );
        REQUIRE( m1_right[1][0] == 8 );
        REQUIRE( m1_right[2][0] == 6 );
        REQUIRE( m1_right[3][0] == 8 );
        REQUIRE( m1_bottom[0][0] == 3 );
        REQUIRE( m1_bottom[0][1] == 6 );
        REQUIRE( m1_bottom[1][0] == 7 );
        REQUIRE( m1_bottom[1][1] == 8 );
    }
    SECTION( "ostream operator gives a human readable representation of the elements" ) {
        Matrix<int> m1({
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
    SECTION( "rows() returns the number of matrix rows" ) {
        Matrix<int> m1({
            { 1, 2 },
            { 9, 8 },
            { 2, 8 }
        });

        REQUIRE( m1.rows() == 3 );
    }
    SECTION( "cols() returns the number of matrix columns" ) {
        Matrix<int> m1({
            { 1, 2, 3, 6 },
            { 9, 8, 3, 9 },
        });

        REQUIRE( m1.cols() == 4 );
    }
    SECTION( "step() returns the number of bytes per row" ) {
        // simulate padded matrix data
        unsigned char bytes[] = {
            9, 4, 5, 0, 0,
            3, 5, 7, 0, 0,
            2, 4, 6, 0, 0
        };
        Matrix<unsigned char> m1(bytes, 3, 3, sizeof(bytes[0]) * 3);

        REQUIRE( m1.rows() == 3 );
        REQUIRE( m1.cols() == 3 );
        REQUIRE( m1.step() == 3 );
    }
    SECTION( "size() returns size of the matrix including padding" ) {
        // simulate padded matrix data
        unsigned char bytes[] = {
            9, 4, 5, 0, 0,
            3, 5, 7, 0, 0,
            2, 4, 6, 0, 0,
            1, 1, 1, 0, 0,
            0, 0, 0, 0, 0
        };
        Matrix<unsigned char> m1(bytes, 5, 3, sizeof(bytes[0]) * 3);

        REQUIRE( m1.size() == 5 * 3 );
    }
    SECTION( "bytes() returns the address of the first matrix element in memory" ) {
        unsigned char bytes[] = {
            9, 4,
            2, 8
        };
        Matrix<unsigned char> m1(bytes, 2, 2);

        REQUIRE( m1.bytes() == bytes );
        REQUIRE( m1.bytes() + 1 == bytes + 1 );
    }
    SECTION( "data() returns the address of the first matrix element in memory" ) {
        uint16_t values[] = {
            9, 4,
            2, 8
        };
        Matrix<uint16_t> m1(values, 2, 2);

        REQUIRE( m1.data() == values );
        REQUIRE( m1.data() + 1 == values + 1 );
        REQUIRE( reinterpret_cast<uintptr_t>(m1.data()) == reinterpret_cast<uintptr_t>(m1.bytes()) );
        REQUIRE( reinterpret_cast<uintptr_t>(m1.data() + 1) != reinterpret_cast<uintptr_t>(m1.bytes() + 1) );
    }
    SECTION( "owns_data() returns true if the instance owns the allocated memory" ) {
        unsigned char *bytes = new unsigned char[4] {
            9, 4,
            2, 8
        };
        Matrix<unsigned char> m1(bytes, 2, 2);
        // we pass ownership of bytes to m2 here, meaning m2's dtor will clean it up once it goes
        // out of scope
        Matrix<unsigned char> m2(bytes, 2, 2, 0, true);
        Matrix<unsigned char> m3(6, 7);
        Matrix<unsigned char> m4;
        m1.copy(m4);

        REQUIRE( m1.owns_data() == false );
        REQUIRE( m2.owns_data() == true );
        REQUIRE( m3.owns_data() == true );
        REQUIRE( m4.owns_data() == true );
    }
    SECTION( "empty() returns true only for empty, non-allocated matrices" ) {
        Matrix<int> m1;
        Matrix<int> m2(2, 3);

        REQUIRE( m1.empty() );
        REQUIRE( !m2.empty() );
    }
    SECTION( "clear() deletes all elements and frees any allocated memory" ) {
        int elems[] = { 3, 4 };
        Matrix<int> m1(5, 3);
        Matrix<int> m2(elems, 1, 2);

        REQUIRE( !m1.empty() );
        REQUIRE( !m2.empty() );
        REQUIRE( m1.size() == 5 * 3 * sizeof(int) );
        REQUIRE( m2.size() == 2 * sizeof(elems[0]) );
        REQUIRE( m1.data() != nullptr );
        REQUIRE( m2.data() != nullptr );
        REQUIRE( m2.data() == elems );

        m1.clear();
        m2.clear();

        REQUIRE( m1.empty() );
        REQUIRE( m2.empty() );
        REQUIRE( m1.size() == 0 );
        REQUIRE( m2.size() == 0 );
        REQUIRE( m1.data() == nullptr );
        REQUIRE( m2.data() == nullptr );
    }
    SECTION( "reserve() only performs reallocation if the capacity grows" ) {
        Matrix<int> m1(5, 3);
        int *m1_elems = m1.data();

        m1.reserve(4, 3);

        REQUIRE( m1.data() == m1_elems );

        m1.reserve(6, 6);

        REQUIRE( m1.data() != m1_elems );
    }
    SECTION( "reserve() only changes the size if the capacity grows" ) {
        Matrix<int> m1(5, 3);
        size_t m1_rows = m1.rows();
        size_t m1_cols = m1.cols();
        m1.reserve(3, 3);

        REQUIRE( m1.rows() == m1_rows );
        REQUIRE( m1.cols() == m1_cols );

        m1.reserve(7, 7);

        REQUIRE( m1.rows() == 0 );
        REQUIRE( m1.cols() == 0 );
    }
    SECTION( "resize() only performs reallocation if the size has changed" ) {
        Matrix<int> m1(5, 3);
        Matrix<int> m2;
        int *m1_elems = m1.data();

        REQUIRE( m1.size() != m2.size() );

        m1.resize(5, 3);
        m2.resize(m1.rows(), m1.cols());

        REQUIRE( m1.data() == m1_elems );
        REQUIRE( m1.size() == m2.size() );
    }
    SECTION( "reshape() reshapes the matrix" ) {
        int values[] = {
            9, 4, 5,
            3, 5, 7,
            2, 4, 6,
            1, 1, 1,
        };
        Matrix<int> m1(values, 4, 3, sizeof(values[0]) * 3);

        REQUIRE( m1.rows() == 4 );
        REQUIRE( m1.cols() == 3 );

        m1.reshape(3, 4);

        REQUIRE( m1.rows() == 3 );
        REQUIRE( m1.cols() == 4 );
        REQUIRE( m1[0][0] == 9 );
        REQUIRE( m1[0][1] == 4 );
        REQUIRE( m1[0][2] == 5 );
        REQUIRE( m1[0][3] == 3 );
        REQUIRE( m1[1][0] == 5 );
        REQUIRE( m1[1][1] == 7 );
        REQUIRE( m1[1][2] == 2 );
        REQUIRE( m1[1][3] == 4 );
        REQUIRE( m1[2][0] == 6 );
        REQUIRE( m1[2][1] == 1 );
        REQUIRE( m1[2][2] == 1 );
        REQUIRE( m1[2][3] == 1 );
    }
    SECTION( "copy() copies matrix elements into another instance" ) {
        Matrix<int> m1({
            { 3, 4 },
            { 1, 9 },
            { 4, 5 }
        });
        Matrix<int> m2;

        REQUIRE( !m1.empty() );
        REQUIRE( m2.empty() );
        REQUIRE( m1.size() == 3 * 2 * sizeof(int) );
        REQUIRE( m2.size() == 0 );

        m1.copy(m2);

        REQUIRE( m2.data() != m1.data() );
        REQUIRE( !m1.empty() );
        REQUIRE( !m2.empty() );
        REQUIRE( m1.size() == 3 * 2 * sizeof(int) );
        REQUIRE( m2.size() == m1.size() );
    }
    SECTION( "move() moves matrix elements into another instance" ) {
        Matrix<int> m1({
            { 3, 4 },
            { 1, 9 },
            { 4, 5 }
        });
        Matrix<int> m2;

        REQUIRE( !m1.empty() );
        REQUIRE( m2.empty() );
        REQUIRE( m1.size() == 3 * 2 * sizeof(int) );
        REQUIRE( m2.size() == 0 );

        m1.move(m2);

        REQUIRE( m1.empty() );
        REQUIRE( !m2.empty() );
        REQUIRE( m1.size() == 0 );
        REQUIRE( m2.size() == 3 * 2 * sizeof(int) );
    }
}
