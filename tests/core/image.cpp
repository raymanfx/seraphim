#include <catch2/catch.hpp>

#include <seraphim/core/image.h>

using namespace sph;

TEST_CASE( "Image constructor", "[Image]" ) {
    SECTION( "default constructor creates empty instances" ) {
        Image i1;

        REQUIRE( i1.empty() );
    }
    SECTION( "image data can be assigned from arbitrary sources (zero-copy)" ) {
        // 3x3 grayscale image
        unsigned char bytes1[] = {
            10, 50, 10,
            50, 100, 50,
            10, 50, 10
        };
        // 2x4 bgr image, rows aligned on word boundaries
        unsigned char bytes2[] = {
            10, 4, 20, 0, 100, 1, /* padding */ 0, 0,
            3, 100, 9, 100, 0, 7, /* padding */ 0, 0,
            0, 0, 100, 0, 100, 0, /* padding */ 0, 0,
            255, 0, 0, 0, 0, 255, /* padding */ 0, 0
        };
        Image i1(bytes1, 3, 3, Pixelformat::Enum::GRAY8);
        Image i2(bytes2, 2, 4, Pixelformat::Enum::RGB24, 8);

        REQUIRE( i1.pixel(0, 0)[0] == 10 );
        REQUIRE( i1.pixel(1, 0)[0] == 50 );
        REQUIRE( i1.pixel(2, 0)[0] == 10 );
        REQUIRE( i1.pixel(0, 1)[0] == 50 );
        REQUIRE( i1.pixel(1, 1)[0] == 100 );
        REQUIRE( i1.pixel(2, 1)[0] == 50 );
        REQUIRE( i1.pixel(0, 2)[0] == 10 );
        REQUIRE( i1.pixel(1, 2)[0] == 50 );
        REQUIRE( i1.pixel(2, 2)[0] == 10 );

        REQUIRE( i2.pixel(0, 0)[0] == 10 );
        REQUIRE( i2.pixel(0, 0)[1] == 4 );
        REQUIRE( i2.pixel(0, 0)[2] == 20 );
        REQUIRE( i2.pixel(1, 0)[0] == 0 );
        REQUIRE( i2.pixel(1, 0)[1] == 100 );
        REQUIRE( i2.pixel(1, 0)[2] == 1 );
        REQUIRE( i2.pixel(0, 1)[0] == 3 );
        REQUIRE( i2.pixel(0, 1)[1] == 100 );
        REQUIRE( i2.pixel(0, 1)[2] == 9 );
        REQUIRE( i2.pixel(1, 1)[0] == 100 );
        REQUIRE( i2.pixel(1, 1)[1] == 0 );
        REQUIRE( i2.pixel(1, 1)[2] == 7 );
        REQUIRE( i2.pixel(0, 2)[0] == 0 );
        REQUIRE( i2.pixel(0, 2)[1] == 0 );
        REQUIRE( i2.pixel(0, 2)[2] == 100 );
        REQUIRE( i2.pixel(1, 2)[0] == 0 );
        REQUIRE( i2.pixel(1, 2)[1] == 100 );
        REQUIRE( i2.pixel(1, 2)[2] == 0 );
        REQUIRE( i2.pixel(0, 3)[0] == 255 );
        REQUIRE( i2.pixel(0, 3)[1] == 0 );
        REQUIRE( i2.pixel(0, 3)[2] == 0 );
        REQUIRE( i2.pixel(1, 3)[0] == 0 );
        REQUIRE( i2.pixel(1, 3)[1] == 0 );
        REQUIRE( i2.pixel(1, 3)[2] == 255 );
    }
}

TEST_CASE( "Image runtime behavior", "[Image]" ) {
    SECTION( "data() returns the address of the pixel data" ) {
        unsigned char bytes[] = {
            1, 2, 3
        };
        Image i1(bytes, 3, 1, Pixelformat::Enum::UNKNOWN);

        REQUIRE( i1.data() == bytes );
    }
    SECTION( "empty() checks whether the image contains pixel data" ) {
        unsigned char bytes[] = {
            1, 2, 3
        };
        Image i1(bytes, 3, 1, Pixelformat::Enum::UNKNOWN);
        Image i2;

        REQUIRE( !i1.empty() );
        REQUIRE( i2.empty() );
    }
    SECTION( "width() returns the number of pixels per row" ) {
        unsigned char bytes[] = {
            1, 2, 3
        };
        Image i1(bytes, 3, 1, Pixelformat::Enum::UNKNOWN);

        REQUIRE( i1.width() == 3 );
    }
    SECTION( "height() returns the number of pixel rows" ) {
        unsigned char bytes[] = {
            1, 2, 3
        };
        Image i1(bytes, 3, 1, Pixelformat::Enum::UNKNOWN);

        REQUIRE( i1.height() == 1 );
    }
    SECTION( "stride() returns the number of bytes per pixel row" ) {
        unsigned char bytes[] = {
            1, 2, 3, 0,
            6, 8, 2, 0
        };
        Image i1(bytes, 3, 2, Pixelformat::Enum::GRAY8);

        REQUIRE( i1.stride() == 3 );
    }
    SECTION( "pixfmt() returns pixelformat of the image" ) {
        Image i1(nullptr, 0, 0, Pixelformat::Enum::BGR24);
        Image i2;

        REQUIRE( i1.pixfmt() == Pixelformat::Enum::BGR24 );
        REQUIRE( i2.pixfmt() == Pixelformat::Enum::UNKNOWN );
    }
    SECTION( "channels() returns the number of channels per pixel" ) {
        Image i1(nullptr, 0, 0, Pixelformat::Enum::BGR32);
        Image i2(nullptr, 0, 0, Pixelformat::Enum::GRAY16);

        REQUIRE( i1.channels() == 3 );
        REQUIRE( i2.channels() == 1 );
    }
    SECTION( "depth() returns the number of bits per pixel" ) {
        Image i1(nullptr, 0, 0, Pixelformat::Enum::GRAY8);
        Image i2(nullptr, 0, 0, Pixelformat::Enum::RGB32);

        REQUIRE( i1.depth() == 8 );
        REQUIRE( i2.depth() == 32 );
    }
    SECTION( "clear() clears the image back buffer" ) {
        unsigned char bytes[] = {
            1, 2, 3
        };
        Image i1(bytes, 3, 1, Pixelformat::Enum::GRAY8);

        REQUIRE( !i1.empty() );
        REQUIRE( i1.data() != nullptr );

        i1.clear();

        REQUIRE( i1.empty() );
        REQUIRE( i1.data() == nullptr );
    }
    SECTION( "validate() checks whether the image contains pixels and has a valid format" ) {
        unsigned char bytes[] = {
            1, 2, 3
        };
        Image i1(bytes, 3, 1, Pixelformat::Enum::GRAY8);
        Image i2(bytes, 3, 1, Pixelformat::Enum::UNKNOWN);
        Image i3(nullptr, 0, 0, Pixelformat::Enum::RGB24);

        REQUIRE( i1.validate() );
        REQUIRE( !i2.validate() );
        REQUIRE( !i3.validate() );
    }
    SECTION( "scanline() returns the address of the pixel row" ) {
        unsigned char bytes[] = {
            1, 2,
            4, 5,
            1, 3
        };
        Image i1(bytes, 2, 3, Pixelformat::Enum::GRAY8);

        REQUIRE( i1.scanline(0) == bytes );
        REQUIRE( i1.scanline(1) == bytes + 2 );
    }
    SECTION( "pixel() returns the address of the pixel" ) {
        unsigned char bytes[] = {
            1, 2,
            4, 5,
            1, 3
        };
        Image i1(bytes, 2, 3, Pixelformat::Enum::GRAY8);

        REQUIRE( i1.pixel(1, 0) == bytes + 1 );
        REQUIRE( i1.pixel(1, 2) == bytes + 5 );
    }
    SECTION( "load() loads pixel data from arbitrary sources" ) {
        unsigned char bytes[] = {
            1, 2,
            4, 5,
            1, 3
        };
        Pixelformat::Enum pixfmt = Pixelformat::Enum::GRAY8;
        ImageConverter::Source src;
        src.buf = bytes;
        src.width = 2;
        src.height = 3;
        src.fourcc = Pixelformat::fourcc(pixfmt);
        src.stride = Pixelformat::bits(pixfmt) * 2;
        Image i1;
        i1.load(src, pixfmt);

        REQUIRE( !i1.empty() );
        REQUIRE( i1.width() == 2 );
        REQUIRE( i1.height() == 3 );
        REQUIRE( i1.pixfmt() == pixfmt );
        REQUIRE( i1.stride() == 2 * 1 );
    }
    SECTION( "convert() converts pixel data into well-defined formats" ) {
        unsigned char bytes[] = {
            1, 2,
            4, 5,
            1, 3
        };
        Image i1(bytes, 2, 3, Pixelformat::Enum::GRAY8);
        bool success = i1.convert(Pixelformat::Enum::BGR32);

        REQUIRE( success );
        REQUIRE( i1.width() == 2 );
        REQUIRE( i1.height() == 3 );
        REQUIRE( i1.pixfmt() == Pixelformat::Enum::BGR32 );
        REQUIRE( i1.stride() == 2 * 4 );
        REQUIRE( i1.width() == 2 );
    }
}
