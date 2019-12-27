#include <catch2/catch.hpp>
#include <sstream>

#include <seraphim/pixel.h>

using namespace sph;

TEST_CASE( "Pixel values can be assigned", "[Pixel]" ) {
    SECTION( "MONO" ) {
        sph::pix::MONO<uint16_t> mono;
        mono.y = 1;

        REQUIRE( mono.y == 1 );
    }
    SECTION( "RGB" ) {
        sph::pix::RGB<uint16_t> rgb;
        rgb.r = 1;
        rgb.g = 2;
        rgb.b = 3;

        REQUIRE( rgb.r == 1 );
        REQUIRE( rgb.g == 2 );
        REQUIRE( rgb.b == 3 );
    }
    SECTION( "RGBA" ) {
        sph::pix::RGB<uint16_t, 4> rgba;
        rgba.r = 1;
        rgba.g = 2;
        rgba.b = 3;
        rgba.data[3] = 4;

        REQUIRE( rgba.r == 1 );
        REQUIRE( rgba.g == 2 );
        REQUIRE( rgba.b == 3 );
        REQUIRE( rgba.data[3] == 4 );
    }
    SECTION( "BGR" ) {
        sph::pix::BGR<uint16_t> bgr;
        bgr.b = 1;
        bgr.g = 2;
        bgr.r = 3;

        REQUIRE( bgr.b == 1 );
        REQUIRE( bgr.g == 2 );
        REQUIRE( bgr.r == 3 );
    }
    SECTION( "BGRA" ) {
        sph::pix::BGR<uint16_t, 4> bgra;
        bgra.b = 1;
        bgra.g = 2;
        bgra.r = 3;
        bgra.data[3] = 4;

        REQUIRE( bgra.b == 1 );
        REQUIRE( bgra.g == 2 );
        REQUIRE( bgra.r == 3 );
        REQUIRE( bgra.data[3] == 4 );
    }
}

TEST_CASE( "Pixel operators", "[Pixel]" ) {
    SECTION( "ostream operator gives a human readable representation of the elements" ) {
        sph::pix::RGB<uint16_t> rgb;
        rgb.r = 1;
        rgb.g = 2;
        rgb.b = 3;

        std::stringstream rgb_strstr;
        std::string rgb_str;
        rgb_strstr << rgb;
        rgb_str = rgb_strstr.str();

        REQUIRE( rgb_str[0] == '[' );
        REQUIRE( rgb_str[1] == '1' );
        REQUIRE( rgb_str[2] == ' ' );
        REQUIRE( rgb_str[3] == '2' );
        REQUIRE( rgb_str[4] == ' ' );
        REQUIRE( rgb_str[5] == '3' );
        REQUIRE( rgb_str[6] == ']' );
    }
}
