/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <algorithm>
#include <cmath>

#include "seraphim/except.h"
#include "seraphim/image.h"
#include "seraphim/image_converter.h"

using namespace sph;

template <class T> static T clamp(T val, T min, T max) {
    if (val <= min) {
        return min;
    } else if (val > max) {
        return max;
    }

    return val;
}

static bool rgb_to_bgr(const Image &src, CoreImage &dst, const Pixelformat &fmt) {
    size_t src_offset;

    uint8_t src_r_off = 0;
    uint8_t src_g_off = 0;
    uint8_t src_b_off = 0;
    uint8_t dst_r_off = 0;
    uint8_t dst_g_off = 0;
    uint8_t dst_b_off = 0;

    switch (src.pixfmt().pattern) {
    case Pixelformat::Pattern::RGB:
        src_r_off = 0;
        src_g_off = 1;
        src_b_off = 2;
        break;
    case Pixelformat::Pattern::BGR:
        src_r_off = 2;
        src_g_off = 1;
        src_b_off = 0;
        break;
    default:
        return false;
    }

    switch (fmt.pattern) {
    case Pixelformat::Pattern::RGB:
        dst_r_off = 0;
        dst_g_off = 1;
        dst_b_off = 2;
        break;
    case Pixelformat::Pattern::BGR:
        dst_r_off = 2;
        dst_g_off = 1;
        dst_b_off = 0;
        break;
    default:
        return false;
    }

    // validate pixel sizes
    if (src.pixfmt().size > 4 || fmt.size > 4) {
        return false;
    }

    CoreImage _dst(src.width(), src.height(), fmt);
    auto data = reinterpret_cast<const unsigned char *>(src.data());

    for (uint32_t y = 0; y < src.height(); y++) {
        for (uint32_t x = 0; x < src.width(); x++) {
            src_offset = y * src.stride() + x * src.pixfmt().size;
            /* each pixel is three bytes */
            _dst.pixel(x, y)[src_r_off] = data[src_offset + dst_r_off];
            _dst.pixel(x, y)[src_g_off] = data[src_offset + dst_g_off];
            _dst.pixel(x, y)[src_b_off] = data[src_offset + dst_b_off];
        }
    }

    dst = std::move(_dst);
    return true;
}

static size_t rgb_to_y(const Image &src, CoreImage &dst, const Pixelformat &fmt) {
    size_t src_offset;

    uint8_t src_r_off = 0;
    uint8_t src_g_off = 0;
    uint8_t src_b_off = 0;

    switch (src.pixfmt().pattern) {
    case Pixelformat::Pattern::RGB:
        src_r_off = 0;
        src_g_off = 1;
        src_b_off = 2;
        break;
    case Pixelformat::Pattern::BGR:
        src_r_off = 2;
        src_g_off = 1;
        src_b_off = 0;
        break;
    default:
        return false;
    }

    switch (fmt.pattern) {
    case Pixelformat::Pattern::MONO:
        break;
    default:
        return false;
    }

    // validate pixel sizes
    if (src.pixfmt().size > 4 || fmt.size > 2) {
        return false;
    }

    CoreImage _dst(src.width(), src.height(), fmt);
    uint8_t r, g, b;
    uint16_t intermediate;
    uint16_t gray16;
    uint16_t max_gray = static_cast<uint16_t>(std::pow(2, fmt.size * 8));

    for (uint32_t y = 0; y < src.height(); y++) {
        for (uint32_t x = 0; x < src.width(); x++) {
            src_offset = y * src.stride() + x * src.pixfmt().size;

            // load the src pixels
            r = *reinterpret_cast<uint8_t *>(src.data() + src_offset + src_r_off);
            g = *reinterpret_cast<uint8_t *>(src.data() + src_offset + src_g_off);
            b = *reinterpret_cast<uint8_t *>(src.data() + src_offset + src_b_off);

            // use weighted (luminosity) method
            // http://www.fourcc.org/fccyvrgb.php
            intermediate = static_cast<uint16_t>(0.299f * r + 0.587f * g + 0.114f * b);
            gray16 = static_cast<uint16_t>(clamp(intermediate, static_cast<uint16_t>(0), max_gray));
            *reinterpret_cast<uint16_t *>(_dst.pixel(x, y)) = gray16;
        }
    }

    dst = std::move(_dst);
    return true;
}

static bool y_to_rgb(const Image &src, CoreImage &dst, const Pixelformat &fmt) {
    size_t src_offset;

    uint8_t dst_r_off = 0;
    uint8_t dst_g_off = 0;
    uint8_t dst_b_off = 0;

    switch (src.pixfmt().pattern) {
    case Pixelformat::Pattern::MONO:
        break;
    default:
        return false;
    }

    switch (fmt.pattern) {
    case Pixelformat::Pattern::RGB:
        dst_r_off = 0;
        dst_g_off = 1;
        dst_b_off = 2;
        break;
    case Pixelformat::Pattern::BGR:
        dst_r_off = 2;
        dst_g_off = 1;
        dst_b_off = 0;
        break;
    default:
        return false;
    }

    // validate pixel sizes
    if (src.pixfmt().size > 2 || fmt.size > 4) {
        return false;
    }

    CoreImage _dst(src.width(), src.height(), fmt);
    uint16_t gray16;

    // https://stackoverflow.com/a/4494004
    for (uint32_t y = 0; y < src.height(); y++) {
        for (uint32_t x = 0; x < src.width(); x++) {
            src_offset = y * src.stride() + x * src.pixfmt().size;

            gray16 = *(reinterpret_cast<const uint16_t *>(src.data() + src_offset));

            _dst.pixel(x, y)[dst_r_off] = static_cast<uint8_t>(
                clamp(gray16, static_cast<uint16_t>(0), static_cast<uint16_t>(255)));
            _dst.pixel(x, y)[dst_g_off] = static_cast<uint8_t>(
                clamp(gray16, static_cast<uint16_t>(0), static_cast<uint16_t>(255)));
            _dst.pixel(x, y)[dst_b_off] = static_cast<uint8_t>(
                clamp(gray16, static_cast<uint16_t>(0), static_cast<uint16_t>(255)));
        }
    }

    dst = std::move(_dst);
    return true;
}

static size_t yuy2_to_rgb(const Image &src, CoreImage &dst, const Pixelformat &fmt) {
    size_t src_offset;

    uint8_t dst_r_off = 0;
    uint8_t dst_g_off = 0;
    uint8_t dst_b_off = 0;

    switch (src.pixfmt().pattern) {
    case Pixelformat::Pattern::YUYV:
        break;
    default:
        return false;
    }

    switch (fmt.pattern) {
    case Pixelformat::Pattern::RGB:
        dst_r_off = 0;
        dst_g_off = 1;
        dst_b_off = 2;
        break;
    case Pixelformat::Pattern::BGR:
        dst_r_off = 2;
        dst_g_off = 1;
        dst_b_off = 0;
        break;
    default:
        return false;
    }

    // validate pixel sizes
    if (src.pixfmt().size > 2 || fmt.size > 4) {
        return false;
    }

    // YUY2 requires the width to be even because two image pixels make a macropixel and a pixel
    // row may only contain full macropixels
    if (src.width() % 2 != 0) {
        return false;
    }

    CoreImage _dst(src.width(), src.height(), fmt);
    auto data = reinterpret_cast<const unsigned char *>(src.data());
    uint8_t y0, u0, y1, v0;
    int8_t c, d, e;

    // https://stackoverflow.com/a/4494004
    for (uint32_t y = 0; y < src.height(); y++) {
        for (uint32_t x = 0; x < src.width(); x += 2) {
            src_offset = y * src.stride() + x * src.pixfmt().size;

            /* each pixel is two bytes, each macropixel (YUYV) is two image pixels */
            y0 = (data + src_offset)[0];
            u0 = (data + src_offset)[1];
            y1 = (data + src_offset)[2];
            v0 = (data + src_offset)[3];
            c = static_cast<int8_t>(y0 - 16);
            d = static_cast<int8_t>(u0 - 128);
            e = static_cast<int8_t>(v0 - 128);

            // the first RGB pixel
            _dst.pixel(x, y)[dst_r_off] =
                static_cast<uint8_t>(clamp(((298 * c + 409 * e + 128) >> 8), 0, 255)); // r
            _dst.pixel(x, y)[dst_g_off] = static_cast<uint8_t>(
                clamp(((298 * c - 100 * d - 208 * e + 128) >> 8), 0, 255)); // g
            _dst.pixel(x, y)[dst_b_off] =
                static_cast<uint8_t>(clamp(((298 * c + 516 * d + 128) >> 8), 0, 255)); // b

            // the second RGB pixel
            c = static_cast<int8_t>(y1 - 16);
            _dst.pixel(x + 1, y)[dst_r_off] =
                static_cast<uint8_t>(clamp(((298 * c + 409 * e + 128) >> 8), 0, 255)); // r
            _dst.pixel(x + 1, y)[dst_g_off] = static_cast<uint8_t>(
                clamp(((298 * c - 100 * d - 208 * e + 128) >> 8), 0, 255)); // g
            _dst.pixel(x + 1, y)[dst_b_off] =
                static_cast<uint8_t>(clamp(((298 * c + 516 * d + 128) >> 8), 0, 255)); // b
        }
    }

    dst = std::move(_dst);
    return true;
}

ImageConverter::ImageConverter() {
    Converter rgb_bgr;
    rgb_bgr.src = { Pixelformat::Pattern::RGB, Pixelformat::Pattern::BGR };
    rgb_bgr.dst = { Pixelformat::Pattern::RGB, Pixelformat::Pattern::BGR };
    rgb_bgr.function = rgb_to_bgr;

    Converter rgb_y;
    rgb_y.src = { Pixelformat::Pattern::RGB, Pixelformat::Pattern::BGR };
    rgb_y.dst = { Pixelformat::Pattern::MONO };
    rgb_y.function = rgb_to_y;

    Converter y_rgb;
    y_rgb.src = { Pixelformat::Pattern::MONO };
    y_rgb.dst = { Pixelformat::Pattern::RGB, Pixelformat::Pattern::BGR };
    y_rgb.function = y_to_rgb;

    Converter yuy2_rgb;
    yuy2_rgb.src = { Pixelformat::Pattern::YUYV };
    yuy2_rgb.dst = { Pixelformat::Pattern::RGB, Pixelformat::Pattern::BGR };
    yuy2_rgb.function = yuy2_to_rgb;

    register_converter(rgb_bgr, 0 /* prio */);
    register_converter(rgb_y, 0 /* prio */);
    register_converter(y_rgb, 0 /* prio */);
    register_converter(yuy2_rgb, 0 /* prio */);
}

bool ImageConverter::convert(const Image &src, CoreImage &dst, const sph::Pixelformat &fmt) {
    Converter conv = {};
    int prio = -1;

    for (const auto &candidate : m_converters) {
        // check source and target format support
        if (std::find(candidate.second.src.begin(), candidate.second.src.end(),
                      src.pixfmt().pattern) != candidate.second.src.end() &&
            std::find(candidate.second.dst.begin(), candidate.second.dst.end(), fmt.pattern) !=
                candidate.second.dst.end()) {
            if (!conv.function || prio < candidate.first) {
                conv = candidate.second;
                prio = candidate.first;
            }
        }
    }

    if (!conv.function) {
        SPH_THROW(LogicException, "No converter for pixelformat");
    }

    return conv.function(src, dst, fmt);
}
