#include <catch2/catch.hpp>

#include <seraphim/image.h>
#include <seraphim/image_converter.h>

using namespace sph;

template <class T> static T clamp(const T &val, const T &min, const T &max) {
    if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    }

    return val;
}

TEST_CASE( "Image conversions", "[ImageConverter]" ) {
    SECTION( "BGR -> RGB" ) {
        // 3x3 BGR24 image
        unsigned char bytes[] = {
            10, 50, 12, 42, 53, 1, 51, 75, 1,
            50, 100, 50, 243, 3, 6, 1, 64, 8,
            10, 50, 16, 43, 42, 7, 88, 2, 44
        };
        BufferedImage i1(bytes, 3, 3, Pixelformat::Enum::BGR24);
        bool success = ImageConverter::Instance().convert(i1, i1, Pixelformat::Enum::RGB32);

        REQUIRE( success );
        REQUIRE( i1.stride() == 3 /* width */ * 4 /* bpp * 8 */ );
        // validate the last pixel row
        REQUIRE( i1.pixel(0, 2)[0] == 16 );
        REQUIRE( i1.pixel(0, 2)[1] == 50 );
        REQUIRE( i1.pixel(0, 2)[2] == 10 );
        REQUIRE( i1.pixel(1, 2)[0] == 7 );
        REQUIRE( i1.pixel(1, 2)[1] == 42 );
        REQUIRE( i1.pixel(1, 2)[2] == 43 );
        REQUIRE( i1.pixel(2, 2)[0] == 44 );
        REQUIRE( i1.pixel(2, 2)[1] == 2 );
        REQUIRE( i1.pixel(2, 2)[2] == 88 );
    }
    SECTION( "RGB -> BGR" ) {
        // 3x3 RGB32 image
        unsigned char bytes[] = {
            10, 50, 12, 0, 42, 53, 1, 0, 51, 75, 1, 0,
            50, 100, 50, 0, 243, 3, 6, 0, 1, 64, 8, 0,
            10, 50, 16, 0, 43, 42, 7, 0, 88, 2, 44, 0
        };
        BufferedImage i1(bytes, 3, 3, Pixelformat::Enum::RGB32);
        bool success = ImageConverter::Instance().convert(i1, i1, Pixelformat::Enum::BGR24);

        REQUIRE( success );
        REQUIRE( i1.stride() == 3 /* width */ * 3 /* bpp * 8 */ );
        // validate the last pixel row
        REQUIRE( i1.pixel(0, 2)[0] == 16 );
        REQUIRE( i1.pixel(0, 2)[1] == 50 );
        REQUIRE( i1.pixel(0, 2)[2] == 10 );
        REQUIRE( i1.pixel(1, 2)[0] == 7 );
        REQUIRE( i1.pixel(1, 2)[1] == 42 );
        REQUIRE( i1.pixel(1, 2)[2] == 43 );
        REQUIRE( i1.pixel(2, 2)[0] == 44 );
        REQUIRE( i1.pixel(2, 2)[1] == 2 );
        REQUIRE( i1.pixel(2, 2)[2] == 88 );
    }
    SECTION( "BGR/RGB -> Y" ) {
        // 3x3 BGR24 image
        unsigned char bytes[] = {
            10, 50, 12, 42, 53, 1, 51, 75, 1,
            50, 100, 50, 243, 3, 6, 1, 64, 8,
            10, 50, 16, 43, 42, 7, 88, 2, 44
        };
        BufferedImage i1(bytes, 3, 3, Pixelformat::Enum::BGR24);
        bool success = ImageConverter::Instance().convert(i1, i1, Pixelformat::Enum::GRAY8);

        REQUIRE( success );
        REQUIRE( i1.stride() == 3 /* width */ * 1 /* bpp * 8 */);

        for (uint32_t i = 0; i < i1.height(); i++) {
            for (uint32_t j = 0; j < i1.width(); j++) {
                unsigned char b, g, r;
                unsigned char y8;
                b = bytes[i /* row */ * 9 /* stride */ + j /* column */ * 3 + 0];
                g = bytes[i /* row */ * 9 /* stride */ + j /* column */ * 3 + 1];
                r = bytes[i /* row */ * 9 /* stride */ + j /* column */ * 3 + 2];
                y8 = static_cast<uint8_t>(
                            clamp(0.299f * r + 0.587f * g + 0.114f * b, 0.0f, 255.0f));

                REQUIRE( i1.pixel(j, i)[0] == y8 );
            }
        }
    }
}
