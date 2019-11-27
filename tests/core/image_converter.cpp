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
        CoreImage i1(bytes, 3, 3, Pixelformat::Enum::BGR24);
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
        CoreImage i1(bytes, 3, 3, Pixelformat::Enum::RGB32);
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
        CoreImage i1(bytes, 3, 3, Pixelformat::Enum::BGR24);
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
    SECTION( "YUYV -> RGB" ) {
        // 2x3 YUYV image
        unsigned char bytes[] = {
            41, 110, 41, 189, 53, 1, 53, 75,
            50, 100, 50, 243, 3, 6, 167, 64,
            100, 50, 16, 43, 42, 47, 88, 62
        };
        Pixelformat yuyv (Pixelformat::Pattern::YUYV, 2);
        CoreImage i1(bytes, 2, 3, yuyv);
        bool success = ImageConverter::Instance().convert(i1, i1, Pixelformat::Enum::RGB24);

        REQUIRE( success );
        REQUIRE( i1.stride() == 2 /* width */ * 3 /* 24bpp */);

        for (uint32_t i = 0; i < i1.height(); i++) {
            for (uint32_t j = 0; j < i1.width(); j += 2) {
                uint8_t r1, g1, b1;
                uint8_t r2, g2, b2;
                uint8_t y0, u0, y1, v0;
                int8_t c, d, e;

                y0 = bytes[i /* row */ * 4 /* stride */ + j /* column */ * 2];
                u0 = bytes[i /* row */ * 4 /* stride */ + j /* column */ * 2 + 1];
                y1 = bytes[i /* row */ * 4 /* stride */ + j /* column */ * 2 + 2];
                v0 = bytes[i /* row */ * 4 /* stride */ + j /* column */ * 2 + 3];

                c = static_cast<int8_t>(y0 - 16);
                d = static_cast<int8_t>(u0 - 128);
                e = static_cast<int8_t>(v0 - 128);

                // the first RGB pixel
                r1 =
                    static_cast<uint8_t>(clamp(((298 * c + 409 * e + 128) >> 8), 0, 255));
                g1 = static_cast<uint8_t>(
                    clamp(((298 * c - 100 * d - 208 * e + 128) >> 8), 0, 255));
                b1 =
                    static_cast<uint8_t>(clamp(((298 * c + 516 * d + 128) >> 8), 0, 255));

                // the second RGB pixel
                c = static_cast<int8_t>(y1 - 16);
                r2 =
                    static_cast<uint8_t>(clamp(((298 * c + 409 * e + 128) >> 8), 0, 255));
                g2 = static_cast<uint8_t>(
                    clamp(((298 * c - 100 * d - 208 * e + 128) >> 8), 0, 255));
                b2 =
                    static_cast<uint8_t>(clamp(((298 * c + 516 * d + 128) >> 8), 0, 255));

                REQUIRE( i1.pixel(j, i)[0] == r1 );
                REQUIRE( i1.pixel(j, i)[1] == g1 );
                REQUIRE( i1.pixel(j, i)[2] == b1 );
                REQUIRE( i1.pixel(j + 1, i)[0] == r2 );
                REQUIRE( i1.pixel(j + 1, i)[1] == g2 );
                REQUIRE( i1.pixel(j + 1, i)[2] == b2 );
            }
        }
    }
}
