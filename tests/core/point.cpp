#include <catch2/catch.hpp>

#include <seraphim/point.h>

using namespace sph;

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

TEST_CASE( "Point2 constructor", "[Point2<T>]" ) {
    SECTION( "number of parameters is respected" ) {
        Point2<int> p1(3, 4);

        REQUIRE( p1.coords().size() == 2 );
    }
    SECTION( "parameters are assigned to internal array" ) {
        Point2<float> p1(2.0f, 3.0f);

        REQUIRE( p1.coords().at(0) == 2.0f );
        REQUIRE( p1.coords().at(1) == 3.0f );
    }
    SECTION( "copy constructor initializes internal array" ) {
        Point2<float> p1(2.0f, 3.0f);
        Point2<float> p1_copy(p1);

        REQUIRE( p1_copy.coords().size() == p1.coords().size() );
        REQUIRE( p1_copy.coords().at(0) == p1.coords().at(0) );
        REQUIRE( p1_copy.coords().at(1) == p1.coords().at(1) );
    }
}

TEST_CASE( "Point2 runtime behavior", "[Point2<T>]" ) {
    SECTION( "empty instances can take coordinates at a later point" ) {
        Point2<double> p1;
        p1[0] = 9.0;
        p1[1] = 7.0;

        REQUIRE( p1.coords().at(0) == 9.0 );
        REQUIRE( p1.coords().at(1) == 7.0 );
    }
    SECTION( "array subscript operator returns correct values" ) {
        Point2<int> p1(2, 3);

        REQUIRE( p1[0] == 2 );
        REQUIRE( p1[1] == 3 );
    }
    SECTION( "member reference aliases work correctly" ) {
        Point2<int> p1(2, 3);
        p1.x += 4;
        p1.y += 1;

        REQUIRE( p1.x == 6 );
        REQUIRE( p1.y == 4 );
    }
    SECTION( "assignment operator populates coordinate array" ) {
        Point2<float> p1(2.0f, 3.0f);
        Point2<float> p1_copy = p1;

        REQUIRE( p1_copy.coords().size() == p1.coords().size() );
        REQUIRE( p1_copy.coords().at(0) == p1.coords().at(0) );
        REQUIRE( p1_copy.coords().at(1) == p1.coords().at(1) );
    }
}

TEST_CASE( "Point3 constructor", "[Point3<T>]" ) {
    SECTION( "number of parameters is respected" ) {
        Point3<int> p1(3, 4, 7);

        REQUIRE( p1.coords().size() == 3 );
    }
    SECTION( "parameters are assigned to internal array" ) {
        Point3<float> p1(2.0f, 3.0f, 5.0f);

        REQUIRE( p1.coords().at(0) == 2.0f );
        REQUIRE( p1.coords().at(1) == 3.0f );
        REQUIRE( p1.coords().at(2) == 5.0f );
    }
    SECTION( "copy constructor initializes internal array" ) {
        Point3<float> p1(2.0f, 3.0f, 5.0f);
        Point3<float> p1_copy(p1);

        REQUIRE( p1_copy.coords().size() == p1.coords().size() );
        REQUIRE( p1_copy.coords().at(0) == p1.coords().at(0) );
        REQUIRE( p1_copy.coords().at(1) == p1.coords().at(1) );
        REQUIRE( p1_copy.coords().at(2) == p1.coords().at(2) );
    }
}

TEST_CASE( "Point3 runtime behavior", "[Point3<T>]" ) {
    SECTION( "empty instances can take coordinates at a later point" ) {
        Point3<double> p1;
        p1[0] = 9.0;
        p1[1] = 7.0;
        p1[2] = 3.0;

        REQUIRE( p1.coords().at(0) == 9.0 );
        REQUIRE( p1.coords().at(1) == 7.0 );
        REQUIRE( p1.coords().at(2) == 3.0 );
    }
    SECTION( "array subscript operator returns correct values" ) {
        Point3<int> p1(2, 3, 9);

        REQUIRE( p1[0] == 2 );
        REQUIRE( p1[1] == 3 );
        REQUIRE( p1[2] == 9 );
    }
    SECTION( "member reference aliases work correctly" ) {
        Point3<int> p1(2, 3, 7);
        p1.x += 4;
        p1.y += 1;
        p1.z -= 2;

        REQUIRE( p1.x == 6 );
        REQUIRE( p1.y == 4 );
        REQUIRE( p1.z == 5 );
    }
    SECTION( "assignment operator populates coordinate array" ) {
        Point3<float> p1(2.0f, 3.0f, 5.0f);
        Point3<float> p1_copy = p1;

        REQUIRE( p1_copy.coords().size() == p1.coords().size() );
        REQUIRE( p1_copy.coords().at(0) == p1.coords().at(0) );
        REQUIRE( p1_copy.coords().at(1) == p1.coords().at(1) );
        REQUIRE( p1_copy.coords().at(2) == p1.coords().at(2) );
    }
}
