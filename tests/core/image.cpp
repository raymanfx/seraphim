#include <catch2/catch.hpp>

#include <seraphim/image.h>

using namespace sph;

TEST_CASE( "CoreImage constructor", "[CoreImage]" ) {
    SECTION( "default constructor creates empty instances" ) {
        CoreImage i1;

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
        CoreImage i1(bytes1, 3, 3, Pixelformat::Enum::GRAY8);
        CoreImage i2(bytes2, 2, 4, Pixelformat::Enum::RGB24, 8);

        REQUIRE( i1.data() == bytes1 );
        REQUIRE( i2.data() == bytes2 );

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

TEST_CASE( "CoreImage runtime behavior", "[CoreImage]" ) {
    SECTION( "data() returns the address of the pixel data" ) {
        unsigned char bytes[] = {
            1, 2,
            4, 5,
            1, 3
        };
        CoreImage i1(bytes, 2, 3, Pixelformat::Enum::GRAY8);

        REQUIRE( i1.data(0) == bytes );
        REQUIRE( i1.data(1) == bytes + 2 );
    }
    SECTION( "empty() checks whether the image contains pixel data" ) {
        unsigned char bytes[] = {
            1, 2, 3
        };
        CoreImage i1(bytes, 3, 1, Pixelformat::Enum::GRAY8);
        CoreImage i2;

        REQUIRE( !i1.empty() );
        REQUIRE( i2.empty() );
    }
    SECTION( "width() returns the number of pixels per row" ) {
        unsigned char bytes[] = {
            1, 2, 3
        };
        CoreImage i1(bytes, 3, 1, Pixelformat::Enum::GRAY8);

        REQUIRE( i1.width() == 3 );
    }
    SECTION( "height() returns the number of pixel rows" ) {
        unsigned char bytes[] = {
            1, 2, 3
        };
        CoreImage i1(bytes, 3, 1, Pixelformat::Enum::GRAY8);

        REQUIRE( i1.height() == 1 );
    }
    SECTION( "stride() returns the number of bytes per pixel row" ) {
        unsigned char bytes[] = {
            1, 2, 3, 0,
            6, 8, 2, 0
        };
        CoreImage i1(bytes, 3, 2, Pixelformat::Enum::GRAY8);

        REQUIRE( i1.stride() == 3 );
    }
    SECTION( "pixfmt() returns pixelformat of the image" ) {
        CoreImage i1(nullptr, 0, 0, Pixelformat::Enum::BGR24);
        CoreImage i2;

        REQUIRE( i1.pixfmt() == Pixelformat::Enum::BGR24 );
        REQUIRE( i2.pixfmt() == Pixelformat::Enum::UNKNOWN );
    }
    SECTION( "channels() returns the number of channels per pixel" ) {
        CoreImage i1(nullptr, 0, 0, Pixelformat::Enum::BGR32);
        CoreImage i2(nullptr, 0, 0, Pixelformat::Enum::GRAY16);

        REQUIRE( i1.channels() == 3 );
        REQUIRE( i2.channels() == 1 );
    }
    SECTION( "depth() returns the number of bits per pixel" ) {
        CoreImage i1(nullptr, 0, 0, Pixelformat::Enum::GRAY8);
        CoreImage i2(nullptr, 0, 0, Pixelformat::Enum::RGB32);

        REQUIRE( i1.depth() == 8 );
        REQUIRE( i2.depth() == 32 );
    }
    SECTION( "clear() clears the image back buffer" ) {
        unsigned char bytes[] = {
            1, 2, 3
        };
        CoreImage i1(bytes, 3, 1, Pixelformat::Enum::GRAY8);

        REQUIRE( !i1.empty() );
        REQUIRE( i1.data() != nullptr );

        i1.clear();

        REQUIRE( i1.empty() );
        REQUIRE( i1.data() == nullptr );
    }
    SECTION( "valid() checks whether the image contains pixels and has a valid format" ) {
        unsigned char bytes[] = {
            1, 2, 3
        };
        CoreImage i1(bytes, 3, 1, Pixelformat::Enum::GRAY8);
        CoreImage i2(nullptr, 0, 0, Pixelformat::Enum::RGB24);

        REQUIRE( i1.valid() );
        REQUIRE( !i2.valid() );
    }
    SECTION( "pixel() returns the address of the pixel" ) {
        unsigned char bytes[] = {
            1, 2,
            4, 5,
            1, 3
        };
        CoreImage i1(bytes, 2, 3, Pixelformat::Enum::GRAY8);

        REQUIRE( i1.pixel(1, 0) == bytes + 1 );
        REQUIRE( i1.pixel(1, 2) == bytes + 5 );
    }
    SECTION( "size() returns the image size in bytes" ) {
        unsigned char bytes[] = {
            1, 2,
            4, 5,
            1, 3
        };
        CoreImage i1(bytes, 2, 3, Pixelformat::Enum::GRAY8);

        REQUIRE( i1.size() == sph::Size2s(2, 3) );
    }
    SECTION( "resize() resizes the image" ) {
        unsigned char bytes[] = {
            1, 2,
            4, 5,
            1, 3
        };
        CoreImage i1(bytes, 2, 3, Pixelformat::Enum::GRAY8);
        i1.resize(2, 1);

        REQUIRE( i1.width() == 2 );
        REQUIRE( i1.height() == 1 );
    }
    SECTION( "forward iterator moves through all elements" ) {
        unsigned char bytes_gray8[] = {
            1, 2,
            4, 5,
            1, 3
        };
        unsigned char bytes_rgb24[] = {
            1, 2, 7, 0,
            4, 5, 9, 0,
            1, 3, 2, 0
        };
        CoreImage i1(bytes_gray8, 2, 3, Pixelformat::Enum::GRAY8);
        CoreImage i2(bytes_rgb24, 1, 3, Pixelformat::Enum::RGB24, 4);

        size_t index = 0;
        for (auto it = i1.begin(); it != i1.end(); ++it) {

            REQUIRE( *it == bytes_gray8 + index );

            index++;
        }

        index = 0;
        for (const auto elem : i2) {

            REQUIRE( elem == bytes_rgb24 + index );

            index += 3;
            if (bytes_rgb24[index] == 0) {
                index++;
            }
        }
    }
}
