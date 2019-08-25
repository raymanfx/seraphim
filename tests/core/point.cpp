#include <catch2/catch.hpp>

#include <seraphim/core/point.h>

using namespace sph::core;

TEST_CASE( "Point constructor", "[Point<T,N>]" ) {
    SECTION( "number of parameters is respected" ) {
        Point<int, 1> p1(3);
        Point<int, 5> p2(3, 4, 5, 6, 7);

        REQUIRE( p1.coords().size() == 1 );
        REQUIRE( p2.coords().size() == 5 );
    }
    SECTION( "parameters are assigned to internal array" ) {
        Point<float, 2> p1(2.0f, 3.0f);

        REQUIRE( p1.coords().at(0) == 2.0f );
        REQUIRE( p1.coords().at(1) == 3.0f );
    }
}

TEST_CASE( "Point runtime behavior", "[Point<T,N>]" ) {
    SECTION( "empty instances can take coordinates at a later point" ) {
        Point<double, 3> p1;
        p1[0] = 9.0;
        p1[1] = 7.0;
        p1[2] = 5.0;

        REQUIRE( p1.coords().at(0) == 9.0 );
        REQUIRE( p1.coords().at(1) == 7.0 );
        REQUIRE( p1.coords().at(2) == 5.0 );
    }
    SECTION( "array subscript operator returns correct values" ) {
        Point<int, 2> p1(2, 3);

        REQUIRE( p1[0] == 2 );
        REQUIRE( p1[1] == 3 );
    }
}
