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
    if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    }

    return val;
}

static bool rgb_to_bgr(const ImageConverter::Source &src, ImageConverter::Target &dst) {
    size_t src_offset;
    CoreImage converted;

    // validate source format
    switch (src.fourcc) {
    case fourcc('B', 'G', 'R', '3'):
    case fourcc('R', 'G', 'B', '3'):
    case fourcc('B', 'G', 'R', '4'):
    case fourcc('R', 'G', 'B', '4'):
        break;
    default:
        return false;
    }

    switch (dst.fmt) {
    case Pixelformat::Enum::BGR24:
    case Pixelformat::Enum::BGR32:
    case Pixelformat::Enum::RGB24:
    case Pixelformat::Enum::RGB32:
        break;
    default:
        return false;
    }

    converted = CoreImage(src.img->width(), src.img->height(), dst.fmt);

    for (uint32_t y = 0; y < src.img->height(); y++) {
        for (uint32_t x = 0; x < src.img->width(); x++) {
            src_offset = y * src.img->stride() + x * src.img->depth() / 8;
            /* each pixel is three bytes */
            auto data = reinterpret_cast<const unsigned char *>(src.img->data());
            converted.pixel(x, y)[0] = data[src_offset + 2];
            converted.pixel(x, y)[1] = data[src_offset + 1];
            converted.pixel(x, y)[2] = data[src_offset + 0];
        }
    }

    *dst.img = std::move(converted);
    return true;
}

static size_t rgb_to_y(const ImageConverter::Source &src, ImageConverter::Target &dst) {
    size_t src_offset;
    CoreImage converted;

    // validate source format
    switch (src.fourcc) {
    case fourcc('B', 'G', 'R', '3'):
    case fourcc('R', 'G', 'B', '3'):
    case fourcc('B', 'G', 'R', '4'):
    case fourcc('R', 'G', 'B', '4'):
        break;
    default:
        return false;
    }

    switch (dst.fmt) {
    case Pixelformat::Enum::GRAY8:
    case Pixelformat::Enum::GRAY16:
        break;
    default:
        return false;
    }

    converted = CoreImage(src.img->width(), src.img->height(), dst.fmt);

    for (uint32_t y = 0; y < src.img->height(); y++) {
        for (uint32_t x = 0; x < src.img->width(); x++) {
            src_offset = y * src.img->stride() + x * src.img->depth() / 8;

            // locate the src.img->data() pixels
            auto data = reinterpret_cast<const unsigned char *>(src.img->data());
            const unsigned char *r, *g, *b;
            switch (src.fourcc) {
            case fourcc('B', 'G', 'R', '3'):
            case fourcc('B', 'G', 'R', '4'):
                r = data + src_offset + 2;
                g = data + src_offset + 1;
                b = data + src_offset + 0;
                break;
            case fourcc('R', 'G', 'B', '3'):
            case fourcc('R', 'G', 'B', '4'):
                r = data + src_offset + 0;
                g = data + src_offset + 1;
                b = data + src_offset + 2;
                break;
            default:
                return false;
            }

            // use weighted (luminosity) method
            // http://www.fourcc.org/fccyvrgb.php
            uint8_t *y8;
            uint16_t *y16;
            switch (dst.fmt) {
            case Pixelformat::Enum::GRAY8:
                y8 = reinterpret_cast<uint8_t *>(converted.pixel(x, y));
                *y8 = static_cast<uint8_t>(
                    clamp(0.299f * *r + 0.587f * *g + 0.114f * *b, 0.0f, 255.0f));
                break;
            case Pixelformat::Enum::GRAY16:
                y16 = reinterpret_cast<uint16_t *>(converted.pixel(x, y));
                *y16 = static_cast<uint16_t>(
                    clamp(0.299f * *r + 0.587f * *g + 0.114f * *b, 0.0f, 65535.0f));
                break;
            default:
                return false;
            }
        }
    }

    *dst.img = std::move(converted);
    return true;
}

static bool y_to_rgb(const ImageConverter::Source &src, ImageConverter::Target &dst) {
    size_t src_offset;
    CoreImage converted;

    // validate source format
    switch (src.fourcc) {
    case fourcc('G', 'R', 'E', 'Y'):
    case fourcc('Y', '1', '6', ' '):
        break;
    default:
        return false;
    }

    switch (dst.fmt) {
    case Pixelformat::Enum::BGR24:
    case Pixelformat::Enum::BGR32:
    case Pixelformat::Enum::RGB24:
    case Pixelformat::Enum::RGB32:
        break;
    default:
        return false;
    }

    converted = CoreImage(src.img->width(), src.img->height(), dst.fmt);

    // https://stackoverflow.com/a/4494004
    for (uint32_t y = 0; y < src.img->height(); y++) {
        for (uint32_t x = 0; x < src.img->width(); x++) {
            src_offset = y * src.img->stride() + x * src.img->depth() / 8;

            auto data = reinterpret_cast<const unsigned char *>(src.img->data());
            uint16_t y16;
            switch (src.fourcc) {
            case fourcc('G', 'R', 'E', 'Y'):
                y16 = *(reinterpret_cast<const uint8_t *>(data + src_offset));
                break;
            case fourcc('Y', '1', '6', ' '):
                y16 = *(reinterpret_cast<const uint16_t *>(data + src_offset));
                break;
            default:
                return false;
            }

            converted.pixel(x, y)[0] = clamp(y16, (uint16_t)0, (uint16_t)255);
            converted.pixel(x, y)[1] = clamp(y16, (uint16_t)0, (uint16_t)255);
            converted.pixel(x, y)[2] = clamp(y16, (uint16_t)0, (uint16_t)255);
        }
    }

    *dst.img = std::move(converted);
    return true;
}

static size_t yuy2_to_rgb(const ImageConverter::Source &src, ImageConverter::Target &dst) {
    size_t src_offset;
    CoreImage converted;

    // validate source format
    switch (src.fourcc) {
    case fourcc('Y', 'U', 'Y', '2'):
    case fourcc('Y', 'U', 'Y', 'V'):
        break;
    default:
        return false;
    }

    switch (dst.fmt) {
    case Pixelformat::Enum::BGR24:
    case Pixelformat::Enum::BGR32:
    case Pixelformat::Enum::RGB24:
    case Pixelformat::Enum::RGB32:
        break;
    default:
        return false;
    }

    converted = CoreImage(src.img->width(), src.img->height(), dst.fmt);

    // https://stackoverflow.com/a/4494004
    for (uint32_t y = 0; y < src.img->height(); y++) {
        for (uint32_t x = 0; x < src.img->width(); x += 2) {
            src_offset = y * src.img->stride() + x * src.img->depth() / 8;

            /* each pixel is two bytes, each macropixel (YUYV) is two image pixels */
            auto data = reinterpret_cast<const unsigned char *>(src.img->data());
            const unsigned char *y0 = data + src_offset + 0;
            const unsigned char *u0 = data + src_offset + 1;
            const unsigned char *y1 = data + src_offset + 2;
            const unsigned char *v0 = data + src_offset + 3;
            uint8_t c = *y0 - 16;
            uint8_t d = *u0 - 128;
            uint8_t e = *v0 - 128;

            // the first RGB pixel
            converted.pixel(x, y)[0] = clamp(((298 * c + 409 * e + 128) >> 8), 0, 255); // r
            converted.pixel(x, y)[1] =
                clamp(((298 * c - 100 * d - 208 * e + 128) >> 8), 0, 255);              // g
            converted.pixel(x, y)[2] = clamp(((298 * c + 516 * d + 128) >> 8), 0, 255); // b

            // the second RGB pixel
            c = *y1 - 16;
            converted.pixel(x + 1, y)[0] = clamp(((298 * c + 409 * e + 128) >> 8), 0, 255); // r
            converted.pixel(x + 1, y)[1] =
                clamp(((298 * c - 100 * d - 208 * e + 128) >> 8), 0, 255);                  // g
            converted.pixel(x + 1, y)[2] = clamp(((298 * c + 516 * d + 128) >> 8), 0, 255); // b

            // swap B/R for BGR
            unsigned char byte;
            switch (dst.fmt) {
            case Pixelformat::Enum::BGR24:
            case Pixelformat::Enum::BGR32:
                // first pixel
                byte = converted.pixel(x, y)[0];
                converted.pixel(x, y)[0] = converted.pixel(x, y)[2];
                converted.pixel(x, y)[2] = byte;
                // second pixel
                byte = converted.pixel(x + 1, y)[0];
                converted.pixel(x + 1, y)[0] = converted.pixel(x + 1, y)[2];
                converted.pixel(x + 1, y)[2] = byte;
                break;
            default:
                break;
            }
        }
    }

    *dst.img = std::move(converted);
    return true;
}

ImageConverter::ImageConverter() {
    Converter rgb_bgr;
    rgb_bgr.source_fmts = { fourcc('B', 'G', 'R', '3'), fourcc('B', 'G', 'R', '4'),
                            fourcc('R', 'G', 'B', '3'), fourcc('R', 'G', 'B', '4') };
    rgb_bgr.target_fmts = { fourcc('B', 'G', 'R', '3'), fourcc('B', 'G', 'R', '4'),
                            fourcc('R', 'G', 'B', '3'), fourcc('R', 'G', 'B', '4') };
    rgb_bgr.function = rgb_to_bgr;

    Converter rgb_y;
    rgb_y.source_fmts = { fourcc('R', 'G', 'B', '3'), fourcc('R', 'G', 'B', '4'),
                          fourcc('B', 'G', 'R', '3'), fourcc('B', 'G', 'R', '4') };
    rgb_y.target_fmts = { fourcc('G', 'R', 'E', 'Y'), fourcc('Y', '1', '6', ' ') };
    rgb_y.function = rgb_to_y;

    Converter y_rgb;
    y_rgb.source_fmts = { fourcc('G', 'R', 'E', 'Y'), fourcc('Y', '1', '6', ' ') };
    y_rgb.target_fmts = { fourcc('B', 'G', 'R', '3'), fourcc('B', 'G', 'R', '4'),
                          fourcc('R', 'G', 'B', '3'), fourcc('R', 'G', 'B', '4') };
    y_rgb.function = y_to_rgb;

    Converter yuy2_rgb;
    yuy2_rgb.source_fmts = { fourcc('Y', 'U', 'Y', '2'), fourcc('Y', 'U', 'Y', 'V') };
    yuy2_rgb.target_fmts = { fourcc('B', 'G', 'R', '3'), fourcc('B', 'G', 'R', '4'),
                             fourcc('R', 'G', 'B', '3'), fourcc('R', 'G', 'B', '4') };
    yuy2_rgb.function = yuy2_to_rgb;

    register_converter(rgb_bgr, 0 /* prio */);
    register_converter(rgb_y, 0 /* prio */);
    register_converter(y_rgb, 0 /* prio */);
    register_converter(yuy2_rgb, 0 /* prio */);
}

static inline bool validate_source(const ImageConverter::Source &src) {
    return src.img && !src.img->empty() && src.fourcc > 0;
}

static inline bool validate_target(const ImageConverter::Target &dst) {
    return dst.img && dst.fmt != Pixelformat::Enum::UNKNOWN;
}

bool ImageConverter::convert(const Source &src, Target &dst) {
    Converter conv = {};
    int prio = -1;

    if (!validate_source(src)) {
        SPH_THROW(LogicException, "Invalid converter source parameters");
    }

    if (!validate_target(dst)) {
        SPH_THROW(LogicException, "Invalid converter target parameters");
    }

    for (const auto &candidate : m_converters) {
        // check source and target format support
        if (std::find(candidate.second.source_fmts.begin(), candidate.second.source_fmts.end(),
                      src.fourcc) != candidate.second.source_fmts.end() &&
            std::find(candidate.second.target_fmts.begin(), candidate.second.target_fmts.end(),
                      Pixelformat::fourcc(dst.fmt)) != candidate.second.target_fmts.end()) {
            if (!conv.function || prio < candidate.first) {
                conv = candidate.second;
            }
        }
    }

    if (!conv.function) {
        SPH_THROW(LogicException, "No converter for pixelformat");
    }

    return conv.function(src, dst);
}

bool ImageConverter::convert(const Image &src, CoreImage &dst, sph::Pixelformat::Enum fmt) {
    Source src_;
    Target dst_;

    if (src.pixfmt() == sph::Pixelformat::Enum::UNKNOWN) {
        SPH_THROW(LogicException, "Source pixelformat must not be unknown");
    }

    src_.img = &src;
    src_.fourcc = Pixelformat::fourcc(src.pixfmt());
    dst_.img = &dst;
    dst_.fmt = fmt;

    return convert(src_, dst_);
}
