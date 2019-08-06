/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <algorithm>
#include <cmath>

#include "seraphim/core/image_buffer_converter.h"

using namespace sph::core;

template <class T> static T clamp(const T &val, const T &min, const T &max) {
    if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    }

    return val;
}

static size_t bgr_to_rgb(unsigned char **src, const ImageBufferConverter::SourceFormat &src_fmt,
                         unsigned char **dst, const ImageBufferConverter::TargetFormat &dst_fmt) {
    size_t src_size;
    size_t src_offset;
    size_t src_pixel_size;
    size_t src_stride;
    size_t dst_size;
    size_t dst_offset;
    size_t dst_pixel_size;
    size_t dst_stride;

    // validate source format
    switch (src_fmt.fourcc) {
    case fourcc('B', 'G', 'R', '3'):
        src_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('B', 'G', 'R', '4'):
        src_pixel_size = 4; /* 32 bpp */
        break;
    default:
        return 0;
    }
    src_stride = (src_fmt.width + src_fmt.padding) * src_pixel_size;
    src_size = src_fmt.height * src_stride;

    // allocate the target buffer
    switch (dst_fmt.fourcc) {
    case fourcc('R', 'G', 'B', '3'):
        dst_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('R', 'G', 'B', '4'):
        dst_pixel_size = 4; /* 32 bpp */
        break;
    default:
        return 0;
    }
    dst_stride = (src_fmt.width + dst_fmt.padding) * dst_pixel_size;
    dst_size = src_fmt.height * dst_stride;

    if (src == dst) {
        // check if in-place conversion is possible
        if (dst_size > src_size) {
            return 0;
        }
    } else {
        // allocate new target buffer
        *dst = new unsigned char[dst_size];
    }

    for (size_t y = 0; y < src_fmt.height; y++) {
        for (size_t x = 0; x < src_fmt.width; x++) {
            // printf("x=%zu, y=%zu\n", x, y);
            src_offset = y * src_stride + x * src_pixel_size;
            dst_offset = y * dst_stride + x * dst_pixel_size;
            /* each pixel is three bytes */
            if (src == dst) {
                unsigned char *tmp = *(src + src_offset + 0);
                *(dst + dst_offset + 0) = *(src + src_offset + 2);
                *(dst + dst_offset + 2) = tmp;
            } else {
                (*dst)[dst_offset + 0] = (*src)[src_offset + 2];
                (*dst)[dst_offset + 1] = (*src)[src_offset + 1];
                (*dst)[dst_offset + 2] = (*src)[src_offset + 0];
            }
        }
    }

    return dst_size;
}

static size_t rgb_to_bgr(unsigned char **src, const ImageBufferConverter::SourceFormat &src_fmt,
                         unsigned char **dst, const ImageBufferConverter::TargetFormat &dst_fmt) {
    size_t src_size;
    size_t src_offset;
    size_t src_pixel_size;
    size_t src_stride;
    size_t dst_size;
    size_t dst_offset;
    size_t dst_pixel_size;
    size_t dst_stride;

    // validate source format
    switch (src_fmt.fourcc) {
    case fourcc('R', 'G', 'B', '3'):
        src_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('R', 'G', 'B', '4'):
        src_pixel_size = 4; /* 32 bpp */
        break;
    default:
        return 0;
    }
    src_stride = (src_fmt.width + src_fmt.padding) * src_pixel_size;
    src_size = src_fmt.height * src_stride;

    // allocate the target buffer
    switch (dst_fmt.fourcc) {
    case fourcc('B', 'G', 'R', '3'):
        dst_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('B', 'G', 'R', '4'):
        dst_pixel_size = 4; /* 32 bpp */
        break;
    default:
        return 0;
    }
    dst_stride = (src_fmt.width + dst_fmt.padding) * dst_pixel_size;
    dst_size = src_fmt.height * dst_stride * dst_pixel_size;

    if (src == dst) {
        // check if in-place conversion is possible
        if (dst_size > src_size) {
            return 0;
        }
    } else {
        // allocate new target buffer
        *dst = new unsigned char[dst_size];
    }

    for (size_t y = 0; y < src_fmt.height; y++) {
        for (size_t x = 0; x < src_fmt.width; x++) {
            src_offset = y * src_stride + x * src_pixel_size;
            dst_offset = y * dst_stride + x * dst_pixel_size;
            /* each pixel is three bytes */
            if (src == dst) {
                unsigned char *tmp = *(src + src_offset + 0);
                *(dst + dst_offset + 0) = *(src + src_offset + 2);
                *(dst + dst_offset + 2) = tmp;
            } else {
                (*dst)[dst_offset + 0] = (*src)[src_offset + 2];
                (*dst)[dst_offset + 1] = (*src)[src_offset + 1];
                (*dst)[dst_offset + 2] = (*src)[src_offset + 0];
            }
        }
    }

    return dst_size;
}

static size_t rgb_to_y16(unsigned char **src, const ImageBufferConverter::SourceFormat &src_fmt,
                         unsigned char **dst, const ImageBufferConverter::TargetFormat &dst_fmt) {
    size_t src_size;
    size_t src_offset;
    size_t src_pixel_size;
    size_t src_stride;
    size_t dst_size;
    size_t dst_offset;
    size_t dst_pixel_size;
    size_t dst_stride;

    // validate source format
    switch (src_fmt.fourcc) {
    case fourcc('B', 'G', 'R', '3'):
    case fourcc('R', 'G', 'B', '3'):
        src_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('B', 'G', 'R', '4'):
    case fourcc('R', 'G', 'B', '4'):
        src_pixel_size = 4; /* 32 bpp */
        break;
    default:
        return 0;
    }
    src_stride = (src_fmt.width + src_fmt.padding) * src_pixel_size;
    src_size = src_fmt.height * src_stride;

    // allocate the target buffer
    switch (dst_fmt.fourcc) {
    case fourcc('Y', '1', '6', ' '):
        dst_pixel_size = 2; /* 16 bpp */
        break;
    default:
        return 0;
    }
    dst_stride = (src_fmt.width + dst_fmt.padding) * dst_pixel_size;
    dst_size = src_fmt.height * dst_stride * dst_pixel_size;

    if (src == dst) {
        // check if in-place conversion is possible
        if (dst_size > src_size) {
            return 0;
        }
    } else {
        // allocate new target buffer
        *dst = new unsigned char[dst_size];
    }

    for (size_t y = 0; y < src_fmt.height; y++) {
        for (size_t x = 0; x < src_fmt.width; x++) {
            src_offset = y * src_stride + x * src_pixel_size;
            dst_offset = y * dst_stride + x * dst_pixel_size;

            // locate the src pixels
            unsigned char *r, *g, *b;
            switch (src_fmt.fourcc) {
            case fourcc('B', 'G', 'R', '3'):
            case fourcc('B', 'G', 'R', '4'):
                r = (*src) + src_offset + 2;
                g = (*src) + src_offset + 1;
                b = (*src) + src_offset + 0;
                break;
            case fourcc('R', 'G', 'B', '3'):
            case fourcc('R', 'G', 'B', '4'):
                r = (*src) + src_offset + 0;
                g = (*src) + src_offset + 1;
                b = (*src) + src_offset + 2;
                break;
            default:
                return 0;
            }

            // use weighted (luminosity) method
            // http://www.fourcc.org/fccyvrgb.php
            int16_t *y = reinterpret_cast<int16_t *>((*dst) + dst_offset);
            *y = static_cast<int16_t>(clamp(0.299f * *r + 0.587f * *g + 0.114f * *b, 0.0f, 255.0f));
        }
    }

    return dst_size;
}

static size_t y16_to_bgr(unsigned char **src, const ImageBufferConverter::SourceFormat &src_fmt,
                         unsigned char **dst, const ImageBufferConverter::TargetFormat &dst_fmt) {
    size_t src_size;
    size_t src_offset;
    size_t src_pixel_size;
    size_t src_stride;
    size_t dst_size;
    size_t dst_offset;
    size_t dst_pixel_size;
    size_t dst_stride;

    // validate source format
    switch (src_fmt.fourcc) {
    case fourcc('Y', '1', '6', ' '):
        src_pixel_size = 2; /* 16 bpp */
        break;
    default:
        return 0;
    }
    src_stride = (src_fmt.width + src_fmt.padding) * src_pixel_size;
    src_size = src_fmt.height * src_stride;

    // allocate the target buffer
    switch (dst_fmt.fourcc) {
    case fourcc('B', 'G', 'R', '3'):
        dst_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('B', 'G', 'R', '4'):
        dst_pixel_size = 4; /* 32 bpp */
        break;
    default:
        return 0;
    }
    dst_stride = (src_fmt.width + dst_fmt.padding) * dst_pixel_size;
    dst_size = src_fmt.height * dst_stride * dst_pixel_size;

    if (src == dst) {
        // check if in-place conversion is possible
        if (dst_size > src_size) {
            return 0;
        }
    } else {
        // allocate new target buffer
        *dst = new unsigned char[dst_size];
    }

    // https://stackoverflow.com/a/4494004
    for (size_t y = 0; y < src_fmt.height; y++) {
        for (size_t x = 0; x < src_fmt.width; x++) {
            src_offset = y * src_stride + x * src_pixel_size;
            dst_offset = y * dst_stride + x * dst_pixel_size;

            int16_t *y = reinterpret_cast<int16_t *>((*src) + src_offset);
            (*dst)[dst_offset + 0] = clamp(*y, (int16_t)0, (int16_t)255); // b
            (*dst)[dst_offset + 1] = clamp(*y, (int16_t)0, (int16_t)255); // g
            (*dst)[dst_offset + 2] = clamp(*y, (int16_t)0, (int16_t)255); // r
        }
    }

    return dst_size;
}

static size_t yuy2_to_bgr(unsigned char **src, const ImageBufferConverter::SourceFormat &src_fmt,
                          unsigned char **dst, const ImageBufferConverter::TargetFormat &dst_fmt) {
    size_t src_size;
    size_t src_offset;
    size_t src_pixel_size;
    size_t src_stride;
    size_t dst_size;
    size_t dst_offset;
    size_t dst_pixel_size;
    size_t dst_stride;

    // validate source format
    switch (src_fmt.fourcc) {
    case fourcc('Y', 'U', 'Y', '2'):
    case fourcc('Y', 'U', 'Y', 'V'):
        src_pixel_size = 4; /* 16 bpp, one macropixel is two pixels */
        break;
    default:
        return 0;
    }
    src_stride = (src_fmt.width + src_fmt.padding) * src_pixel_size;
    src_size = src_fmt.height * src_stride;

    // allocate the target buffer
    switch (dst_fmt.fourcc) {
    case fourcc('B', 'G', 'R', '3'):
        dst_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('B', 'G', 'R', '4'):
        dst_pixel_size = 4; /* 32 bpp */
        break;
    default:
        return 0;
    }
    dst_stride = (src_fmt.width + dst_fmt.padding) * dst_pixel_size;
    dst_size = src_fmt.height * dst_stride * dst_pixel_size;

    if (src == dst) {
        // check if in-place conversion is possible
        if (dst_size > src_size) {
            return 0;
        }
    } else {
        // allocate new target buffer
        *dst = new unsigned char[dst_size];
    }

    // https://stackoverflow.com/a/4494004
    for (size_t y = 0; y < src_fmt.height; y++) {
        for (size_t x = 0; x < src_fmt.width; x++) {
            src_offset = y * src_stride + x * src_pixel_size;
            dst_offset = y * dst_stride + x * dst_pixel_size;
            /* each pixel is two bytes, each macropixel (YUYV) is two image pixels */
            unsigned char *y0 = (*src) + src_offset + 0;
            unsigned char *u0 = (*src) + src_offset + 1;
            unsigned char *y1 = (*src) + src_offset + 2;
            unsigned char *v0 = (*src) + src_offset + 3;
            uint8_t c = *y0 - 16;
            uint8_t d = *u0 - 128;
            uint8_t e = *v0 - 128;
            // the first BGR pixel
            (*dst)[dst_offset + 0] = clamp(((298 * c + 516 * d + 128) >> 8), 0, 255);           // b
            (*dst)[dst_offset + 1] = clamp(((298 * c - 100 * d - 208 * e + 128) >> 8), 0, 255); // g
            (*dst)[dst_offset + 2] = clamp(((298 * c + 409 * e + 128) >> 8), 0, 255);           // r

            if (dst_pixel_size > 3) {
                dst_offset += dst_pixel_size - 3;
            }

            // the second BGR pixel
            c = *y1 - 16;
            (*dst)[dst_offset + 3] = clamp(((298 * c + 516 * d + 128) >> 8), 0, 255);           // b
            (*dst)[dst_offset + 4] = clamp(((298 * c - 100 * d - 208 * e + 128) >> 8), 0, 255); // g
            (*dst)[dst_offset + 5] = clamp(((298 * c + 409 * e + 128) >> 8), 0, 255);           // r
        }
    }

    return dst_size;
}

ImageBufferConverter::ImageBufferConverter() {
    Converter bgr_rgb;
    bgr_rgb.source_fmts = { fourcc('B', 'G', 'R', '3'), fourcc('B', 'G', 'R', '4') };
    bgr_rgb.target_fmts = { fourcc('R', 'G', 'B', '3'), fourcc('R', 'G', 'B', '4') };
    bgr_rgb.function = bgr_to_rgb;

    Converter rgb_bgr;
    rgb_bgr.source_fmts = { fourcc('R', 'G', 'B', '3'), fourcc('R', 'G', 'B', '4') };
    rgb_bgr.target_fmts = { fourcc('B', 'G', 'R', '3'), fourcc('B', 'G', 'R', '4') };
    rgb_bgr.function = rgb_to_bgr;

    Converter rgb_y16;
    rgb_y16.source_fmts = { fourcc('R', 'G', 'B', '3'), fourcc('R', 'G', 'B', '4'),
                            fourcc('B', 'G', 'R', '3'), fourcc('B', 'G', 'R', '4') };
    rgb_y16.target_fmts = { fourcc('Y', '1', '6', ' ') };
    rgb_y16.function = rgb_to_y16;

    Converter y16_bgr;
    y16_bgr.source_fmts = { fourcc('Y', '1', '6', ' ') };
    y16_bgr.target_fmts = { fourcc('B', 'G', 'R', '3'), fourcc('B', 'G', 'R', '4') };
    y16_bgr.function = y16_to_bgr;

    Converter yuy2_bgr;
    yuy2_bgr.source_fmts = { fourcc('Y', 'U', 'Y', '2'), fourcc('Y', 'U', 'Y', 'V') };
    yuy2_bgr.target_fmts = { fourcc('B', 'G', 'R', '3'), fourcc('B', 'G', 'R', '4') };
    yuy2_bgr.function = yuy2_to_bgr;

    register_converter(bgr_rgb, 0 /* prio */);
    register_converter(rgb_bgr, 0 /* prio */);
    register_converter(rgb_y16, 0 /* prio */);
    register_converter(y16_bgr, 0 /* prio */);
    register_converter(yuy2_bgr, 0 /* prio */);
}

size_t ImageBufferConverter::convert(unsigned char **src, const SourceFormat &src_fmt,
                                     unsigned char **dst,
                                     const ImageBufferConverter::TargetFormat &dst_fmt) {
    Converter conv = {};
    int prio = -1;

    for (const auto &candidate : m_converters) {
        // check source and target format support
        if (std::find(candidate.second.source_fmts.begin(), candidate.second.source_fmts.end(),
                      src_fmt.fourcc) != candidate.second.source_fmts.end() &&
            std::find(candidate.second.target_fmts.begin(), candidate.second.target_fmts.end(),
                      dst_fmt.fourcc) != candidate.second.target_fmts.end()) {
            if (!conv.function || prio < candidate.first) {
                conv = candidate.second;
            }
        }
    }

    if (!conv.function) {
        return 0;
    }

    return conv.function(src, src_fmt, dst, dst_fmt);
}
