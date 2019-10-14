#include <catch2/catch.hpp>

#include <seraphim/core/polygon.h>

using namespace sph;

TEST_CASE( "Polygon constructor", "[Polygon<T,N>]" ) {
    SECTION( "number of parameters is respected" ) {
        Polygon<int> p1(Point2i(3, 0));
        Polygon<int> p2(Point2i(3, 0), Point2i(4, 3));

        REQUIRE( p1.points().size() == 1 );
        REQUIRE( p2.points().size() == 2 );
    }
    SECTION( "parameters are assigned to internal vector" ) {
        Polygon<float> p1(Point2f(2.0f, 3.0f));

        REQUIRE( p1.points().at(0).x == 2.0f );
        REQUIRE( p1.points().at(0).y == 3.0f );
    }
}

TEST_CASE( "Polygon runtime behavior", "[Polygon<T,N>]" ) {
    SECTION( "empty instances can take points at a later point in time" ) {
        Polygon<double> p1;
        p1.add_point(Point2d(1.0, 2.0));
        p1.add_point(Point2d(2.0, 3.0));

        REQUIRE( p1.points().size() == 2 );
        REQUIRE( p1.points().at(0).x == 1.0 );
        REQUIRE( p1.points().at(1).y == 3.0 );
    }
}

TEST_CASE( "Polygon extreme points", "[Polygon<T,N>]" ) {
    Polygon<int> p1(Point2i(0, 0), Point2i(2, 0), Point2i(2, 2), Point2i(0, 2));

    SECTION( "top left (tl) point is calculated correctly" ) {
        REQUIRE( p1.tl().x == 0 );
        REQUIRE( p1.tl().y == 0 );
    }
    SECTION( "top right (tr) point is calculated correctly" ) {
        REQUIRE( p1.tr().x == 2 );
        REQUIRE( p1.tr().y == 0 );
    }
    SECTION( "bottom right (br) point is calculated correctly" ) {
        REQUIRE( p1.br().x == 2 );
        REQUIRE( p1.br().y == 2 );
    }
    SECTION( "bottom left (bl) point is calculated correctly" ) {
        REQUIRE( p1.bl().x == 0 );
        REQUIRE( p1.bl().y == 2 );
    }
}

TEST_CASE( "Polygon bounding rectangle", "[Polygon<T,N>]" ) {
    Polygon<int> p1(Point2i(0, 0), Point2i(3, 0), Point2i(2, 2), Point2i(0, 4));

    SECTION( "bounding rectangle width and height are correct" ) {
        REQUIRE( p1.bounding_rect().width() == 3 );
        REQUIRE( p1.bounding_rect().height() == 4 );
    }
    SECTION( "bounding rectangle contains all points" ) {
        REQUIRE( p1.bounding_rect().tl().x == 0 );
        REQUIRE( p1.bounding_rect().tl().y == 0 );
        REQUIRE( p1.bounding_rect().tr().x == 3 );
        REQUIRE( p1.bounding_rect().tr().y == 0 );
        REQUIRE( p1.bounding_rect().br().x == 3 );
        REQUIRE( p1.bounding_rect().br().y == 4 );
        REQUIRE( p1.bounding_rect().bl().x == 0 );
        REQUIRE( p1.bounding_rect().bl().y == 4 );
    }
}
