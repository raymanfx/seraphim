#include <catch2/catch.hpp>

#include <seraphim/polygon.h>

using namespace sph;

TEST_CASE( "Polygon constructor", "[Polygon<T,N>]" ) {
    SECTION( "number of parameters is respected" ) {
        Polygon<int> p1(Point2i(3, 0), Point2i(3, 0), Point2i(3, 0));
        Polygon<int> p2(Point2i(3, 0), Point2i(3, 0), Point2i(3, 0), Point2i(3, 0));

        REQUIRE( p1.vertices().size() == 3 );
        REQUIRE( p2.vertices().size() == 4 );
    }
    SECTION( "parameters are assigned to internal vector" ) {
        Polygon<float> p1(Point2f(2.0f, 3.0f), Point2f(3.0f, 4.0f), Point2f(4.0f, 5.0f));

        REQUIRE( p1.vertices().at(0).x == 2.0f );
        REQUIRE( p1.vertices().at(0).y == 3.0f );
        REQUIRE( p1.vertices().at(1).x == 3.0f );
        REQUIRE( p1.vertices().at(1).y == 4.0f );
        REQUIRE( p1.vertices().at(2).x == 4.0f );
        REQUIRE( p1.vertices().at(2).y == 5.0f );
    }
    SECTION( "container instances are accepted as valid input" ) {
        std::array<Point2f, 3> points = { Point2f(2.0f, 3.0f), Point2f(3.0f, 4.0f), Point2f(4.0f, 5.0f) };
        Polygon<float> p1(points);

        REQUIRE( p1.vertices().at(0).x == 2.0f );
        REQUIRE( p1.vertices().at(0).y == 3.0f );
        REQUIRE( p1.vertices().at(1).x == 3.0f );
        REQUIRE( p1.vertices().at(1).y == 4.0f );
        REQUIRE( p1.vertices().at(2).x == 4.0f );
        REQUIRE( p1.vertices().at(2).y == 5.0f );
    }
}

TEST_CASE( "Polygon runtime behavior", "[Polygon<T,N>]" ) {
    SECTION( "empty() returns the correct state") {
        Polygon<int> p1;
        Polygon<int> p2(Point2i(0, 0), Point2i(3, 0), Point2i(2, 2), Point2i(0, 4));

        REQUIRE( p1.empty() );
        REQUIRE( !p2.empty() );
    }
    SECTION( "vertices() provides access to all points forming the polygon" ) {
        Point2i x1(0, 0);
        Point2i x2(3, 0);
        Point2i x3(2, 2);
        Point2i x4(0, 4);
        Polygon<int> p1(x1, x2, x3, x4);

        REQUIRE( p1.vertices().at(0) == x1 );
        REQUIRE( p1.vertices().at(1) == x2 );
        REQUIRE( p1.vertices().at(2) == x3 );
        REQUIRE( p1.vertices().at(3) == x4 );
    }
    SECTION( "== operator compares instance equality" ) {
        Point2i x1(0, 0);
        Point2i x2(3, 0);
        Point2i x3(2, 2);
        Polygon<int> p1(x1, x2, x3);
        Polygon<int> p2(x1, x2, x3);

        REQUIRE( p1 == p2 );
    }
    SECTION( "!= operator compares instance inequality" ) {
        Point2i x1(0, 0);
        Point2i x2(3, 0);
        Point2i x3(2, 2);
        Polygon<int> p1(x1, x2, x3);
        Polygon<int> p2(x1, x2, x2);

        REQUIRE( p1 != p2 );
    }
    SECTION( "brect() computes the correct bounding rectangle" ) {
        Polygon<int> p1(Point2i(10, 10), Point2i(20, 2), Point2i(15, 90), Point2i(0, 50));

        REQUIRE( p1.brect().tl().x == 0 );
        REQUIRE( p1.brect().tl().y == 2 );
        REQUIRE( p1.brect().tr().x == 20 );
        REQUIRE( p1.brect().tr().y == 2 );
        REQUIRE( p1.brect().br().x == 20 );
        REQUIRE( p1.brect().br().y == 90 );
        REQUIRE( p1.brect().bl().x == 0 );
        REQUIRE( p1.brect().bl().y == 90 );
        REQUIRE( p1.brect().width() == 20 );
        REQUIRE( p1.brect().height() == 88 );
    }
    SECTION( "bounding rectangle contains all vertices" ) {
        Polygon<int> p1(Point2i(0, 0), Point2i(3, 0), Point2i(2, 2), Point2i(0, 4));

        REQUIRE( p1.brect().tl().x == 0 );
        REQUIRE( p1.brect().tl().y == 0 );
        REQUIRE( p1.brect().tr().x == 3 );
        REQUIRE( p1.brect().tr().y == 0 );
        REQUIRE( p1.brect().br().x == 3 );
        REQUIRE( p1.brect().br().y == 4 );
        REQUIRE( p1.brect().bl().x == 0 );
        REQUIRE( p1.brect().bl().y == 4 );
    }
    SECTION( "width() returns the width of the bounding rectangle" ) {
        Polygon<int> p1(Point2i(0, 0), Point2i(3, 0), Point2i(2, 2), Point2i(0, 4));

        REQUIRE( p1.width() == 3 );
    }
    SECTION( "height() returns the height of the bounding rectangle" ) {
        Polygon<int> p1(Point2i(0, 0), Point2i(3, 0), Point2i(2, 2), Point2i(0, 4));

        REQUIRE( p1.height() == 4 );
    }
}
