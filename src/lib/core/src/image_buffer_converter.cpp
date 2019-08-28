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

static bool bgr_to_rgb(const ImageBufferConverter::Source &src, ImageBufferConverter::Target &dst) {
    size_t src_size;
    size_t src_offset;
    size_t src_pixel_size;
    size_t dst_size;
    size_t dst_offset;
    size_t dst_pixel_size;
    size_t dst_padding;
    size_t dst_stride;

    // validate source format
    switch (src.fourcc) {
    case fourcc('B', 'G', 'R', '3'):
        src_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('B', 'G', 'R', '4'):
        src_pixel_size = 4; /* 32 bpp */
        break;
    default:
        return false;
    }

    src_size = src.height * src.stride * src_pixel_size;

    switch (dst.fourcc) {
    case fourcc('R', 'G', 'B', '3'):
        dst_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('R', 'G', 'B', '4'):
        dst_pixel_size = 4; /* 32 bpp */
        break;
    default:
        return false;
    }

    dst_padding = (dst.alignment - ((src.width * dst_pixel_size) % dst.alignment)) % dst.alignment;
    dst_stride = src.width * dst_pixel_size + dst_padding;
    dst_size = src.height * dst_stride;

    // not all formats can be converted in-place
    if (dst.buf == src.buf && src_pixel_size < dst_pixel_size) {
        return false;
    }

    // make sure the target buffer is large enough
    if (dst.buf_len < dst_size) {
        return false;
    }

    for (size_t y = 0; y < src.height; y++) {
        for (size_t x = 0; x < src.width; x++) {
            src_offset = y * src.stride + x * src_pixel_size;
            dst_offset = y * dst_stride + x * dst_pixel_size;
            /* each pixel is three buf */
            if (src.buf == dst.buf) {
                unsigned char tmp = src.buf[src_offset + 0];
                dst.buf[dst_offset + 0] = src.buf[src_offset + 2];
                dst.buf[dst_offset + 2] = tmp;
            } else {
                dst.buf[dst_offset + 0] = src.buf[src_offset + 2];
                dst.buf[dst_offset + 1] = src.buf[src_offset + 1];
                dst.buf[dst_offset + 2] = src.buf[src_offset + 0];
            }
        }
    }

    return true;
}

static bool rgb_to_bgr(const ImageBufferConverter::Source &src, ImageBufferConverter::Target &dst) {
    size_t src_size;
    size_t src_offset;
    size_t src_pixel_size;
    size_t dst_size;
    size_t dst_offset;
    size_t dst_pixel_size;
    size_t dst_padding;
    size_t dst_stride;

    // validate source format
    switch (src.fourcc) {
    case fourcc('R', 'G', 'B', '3'):
        src_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('R', 'G', 'B', '4'):
        src_pixel_size = 4; /* 32 bpp */
        break;
    default:
        return false;
    }

    src_size = src.height * src.stride * src_pixel_size;

    switch (dst.fourcc) {
    case fourcc('B', 'G', 'R', '3'):
        dst_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('B', 'G', 'R', '4'):
        dst_pixel_size = 4; /* 32 bpp */
        break;
    default:
        return false;
    }

    dst_padding = (dst.alignment - ((src.width * dst_pixel_size) % dst.alignment)) % dst.alignment;
    dst_stride = src.width * dst_pixel_size + dst_padding;
    dst_size = src.height * dst_stride;

    // not all formats can be converted in-place
    if (dst.buf == src.buf && src_pixel_size < dst_pixel_size) {
        return false;
    }

    // make sure the target buffer is large enough
    if (dst.buf_len < dst_size) {
        return false;
    }

    for (size_t y = 0; y < src.height; y++) {
        for (size_t x = 0; x < src.width; x++) {
            src_offset = y * src.stride + x * src_pixel_size;
            dst_offset = y * dst_stride + x * dst_pixel_size;
            /* each pixel is three buf */
            if (src.buf == dst.buf) {
                unsigned char tmp = src.buf[src_offset + 0];
                dst.buf[dst_offset + 0] = src.buf[src_offset + 2];
                dst.buf[dst_offset + 2] = tmp;
            } else {
                dst.buf[dst_offset + 0] = src.buf[src_offset + 2];
                dst.buf[dst_offset + 1] = src.buf[src_offset + 1];
                dst.buf[dst_offset + 2] = src.buf[src_offset + 0];
            }
        }
    }

    return true;
}

static size_t rgb_to_y(const ImageBufferConverter::Source &src, ImageBufferConverter::Target &dst) {
    size_t src_size;
    size_t src_offset;
    size_t src_pixel_size;
    size_t dst_size;
    size_t dst_offset;
    size_t dst_pixel_size;
    size_t dst_padding;
    size_t dst_stride;

    // validate source format
    switch (src.fourcc) {
    case fourcc('B', 'G', 'R', '3'):
    case fourcc('R', 'G', 'B', '3'):
        src_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('B', 'G', 'R', '4'):
    case fourcc('R', 'G', 'B', '4'):
        src_pixel_size = 4; /* 32 bpp */
        break;
    default:
        return false;
    }

    src_size = src.height * src.stride * src_pixel_size;

    switch (dst.fourcc) {
    case fourcc('G', 'R', 'E', 'Y'):
        dst_pixel_size = 1; /* 8 bpp */
        break;
    case fourcc('Y', '1', '6', ' '):
        dst_pixel_size = 2; /* 16 bpp */
        break;
    default:
        return false;
    }

    dst_padding = (dst.alignment - ((src.width * dst_pixel_size) % dst.alignment)) % dst.alignment;
    dst_stride = src.width * dst_pixel_size + dst_padding;
    dst_size = src.height * dst_stride;

    // not all formats can be converted in-place
    if (dst.buf == src.buf && src_pixel_size < dst_pixel_size) {
        return false;
    }

    // make sure the target buffer is large enough
    if (dst.buf_len < dst_size) {
        return false;
    }

    for (size_t y = 0; y < src.height; y++) {
        for (size_t x = 0; x < src.width; x++) {
            src_offset = y * src.stride + x * src_pixel_size;
            dst_offset = y * dst_stride + x * dst_pixel_size;

            // locate the src.buf pixels
            unsigned char *r, *g, *b;
            switch (src.fourcc) {
            case fourcc('B', 'G', 'R', '3'):
            case fourcc('B', 'G', 'R', '4'):
                r = src.buf + src_offset + 2;
                g = src.buf + src_offset + 1;
                b = src.buf + src_offset + 0;
                break;
            case fourcc('R', 'G', 'B', '3'):
            case fourcc('R', 'G', 'B', '4'):
                r = src.buf + src_offset + 0;
                g = src.buf + src_offset + 1;
                b = src.buf + src_offset + 2;
                break;
            default:
                return false;
            }

            // use weighted (luminosity) method
            // http://www.fourcc.org/fccyvrgb.php
            uint8_t *y8;
            uint16_t *y16;
            switch (dst.fourcc) {
            case fourcc('G', 'R', 'E', 'Y'):
                y8 = reinterpret_cast<uint8_t *>(dst.buf + dst_offset);
                *y8 = static_cast<uint8_t>(
                    clamp(0.299f * *r + 0.587f * *g + 0.114f * *b, 0.0f, 255.0f));
                break;
            case fourcc('Y', '1', '6', ' '):
                y16 = reinterpret_cast<uint16_t *>(dst.buf + dst_offset);
                *y16 = static_cast<uint16_t>(
                    clamp(0.299f * *r + 0.587f * *g + 0.114f * *b, 0.0f, 65535.0f));
                break;
            default:
                return false;
            }
        }
    }

    return true;
}

static bool y_to_bgr(const ImageBufferConverter::Source &src, ImageBufferConverter::Target &dst) {
    size_t src_size;
    size_t src_offset;
    size_t src_pixel_size;
    size_t dst_size;
    size_t dst_offset;
    size_t dst_pixel_size;
    size_t dst_padding;
    size_t dst_stride;

    // validate source format
    switch (src.fourcc) {
    case fourcc('G', 'R', 'E', 'Y'):
        src_pixel_size = 1; /* 8 bpp */
        break;
    case fourcc('Y', '1', '6', ' '):
        src_pixel_size = 2; /* 16 bpp */
        break;
    default:
        return false;
    }

    src_size = src.height * src.stride * src_pixel_size;

    switch (dst.fourcc) {
    case fourcc('B', 'G', 'R', '3'):
        dst_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('B', 'G', 'R', '4'):
        dst_pixel_size = 4; /* 32 bpp */
        break;
    default:
        return false;
    }

    dst_padding = (dst.alignment - ((src.width * dst_pixel_size) % dst.alignment)) % dst.alignment;
    dst_stride = src.width * dst_pixel_size + dst_padding;
    dst_size = src.height * dst_stride;

    // not all formats can be converted in-place
    if (dst.buf == src.buf && src_pixel_size < dst_pixel_size) {
        return false;
    }

    // make sure the target buffer is large enough
    if (dst.buf_len < dst_size) {
        return false;
    }

    // https://stackoverflow.com/a/4494004
    for (size_t y = 0; y < src.height; y++) {
        for (size_t x = 0; x < src.width; x++) {
            src_offset = y * src.stride + x * src_pixel_size;
            dst_offset = y * dst_stride + x * dst_pixel_size;

            uint16_t y16;
            switch (src.fourcc) {
            case fourcc('G', 'R', 'E', 'Y'):
                y16 = *(reinterpret_cast<uint8_t *>(src.buf + src_offset));
                break;
            case fourcc('Y', '1', '6', ' '):
                y16 = *(reinterpret_cast<uint16_t *>(src.buf + src_offset));
                break;
            default:
                return false;
            }

            dst.buf[dst_offset + 0] = clamp(y16, (uint16_t)0, (uint16_t)255); // b
            dst.buf[dst_offset + 1] = clamp(y16, (uint16_t)0, (uint16_t)255); // g
            dst.buf[dst_offset + 2] = clamp(y16, (uint16_t)0, (uint16_t)255); // r
        }
    }

    return true;
}

static size_t yuy2_to_bgr(const ImageBufferConverter::Source &src,
                          ImageBufferConverter::Target &dst) {
    size_t src_size;
    size_t src_offset;
    size_t src_pixel_size;
    size_t dst_size;
    size_t dst_offset;
    size_t dst_pixel_size;
    size_t dst_padding;
    size_t dst_stride;

    // validate source format
    switch (src.fourcc) {
    case fourcc('Y', 'U', 'Y', '2'):
    case fourcc('Y', 'U', 'Y', 'V'):
        src_pixel_size = 4; /* 16 bpp, one macropixel is two pixels */
        break;
    default:
        return false;
    }

    src_size = src.height * src.stride * src_pixel_size;

    switch (dst.fourcc) {
    case fourcc('B', 'G', 'R', '3'):
        dst_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('B', 'G', 'R', '4'):
        dst_pixel_size = 4; /* 32 bpp */
        break;
    default:
        return false;
    }

    dst_padding = (dst.alignment - ((src.width * dst_pixel_size) % dst.alignment)) % dst.alignment;
    dst_stride = src.width * dst_pixel_size + dst_padding;
    dst_size = src.height * dst_stride;

    // not all formats can be converted in-place
    if (dst.buf == src.buf && src_pixel_size < dst_pixel_size) {
        return false;
    }

    // make sure the target buffer is large enough
    if (dst.buf_len < dst_size) {
        return false;
    }

    // https://stackoverflow.com/a/4494004
    for (size_t y = 0; y < src.height; y++) {
        for (size_t x = 0; x < src.width; x++) {
            src_offset = y * src.stride + x * src_pixel_size;
            dst_offset = y * dst_stride + x * dst_pixel_size;
            /* each pixel is two buf, each macropixel (YUYV) is two image pixels */
            unsigned char *y0 = src.buf + src_offset + 0;
            unsigned char *u0 = src.buf + src_offset + 1;
            unsigned char *y1 = src.buf + src_offset + 2;
            unsigned char *v0 = src.buf + src_offset + 3;
            uint8_t c = *y0 - 16;
            uint8_t d = *u0 - 128;
            uint8_t e = *v0 - 128;
            // the first BGR pixel
            dst.buf[dst_offset + 0] = clamp(((298 * c + 516 * d + 128) >> 8), 0, 255); // b
            dst.buf[dst_offset + 1] =
                clamp(((298 * c - 100 * d - 208 * e + 128) >> 8), 0, 255);             // g
            dst.buf[dst_offset + 2] = clamp(((298 * c + 409 * e + 128) >> 8), 0, 255); // r

            if (dst_pixel_size > 3) {
                dst_offset += dst_pixel_size - 3;
            }

            // the second BGR pixel
            c = *y1 - 16;
            dst.buf[dst_offset + 3] = clamp(((298 * c + 516 * d + 128) >> 8), 0, 255); // b
            dst.buf[dst_offset + 4] =
                clamp(((298 * c - 100 * d - 208 * e + 128) >> 8), 0, 255);             // g
            dst.buf[dst_offset + 5] = clamp(((298 * c + 409 * e + 128) >> 8), 0, 255); // r
        }
    }

    return true;
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

    Converter rgb_y;
    rgb_y.source_fmts = { fourcc('R', 'G', 'B', '3'), fourcc('R', 'G', 'B', '4'),
                          fourcc('B', 'G', 'R', '3'), fourcc('B', 'G', 'R', '4') };
    rgb_y.target_fmts = { fourcc('G', 'R', 'E', 'Y'), fourcc('Y', '1', '6', ' ') };
    rgb_y.function = rgb_to_y;

    Converter y_bgr;
    y_bgr.source_fmts = { fourcc('G', 'R', 'E', 'Y'), fourcc('Y', '1', '6', ' ') };
    y_bgr.target_fmts = { fourcc('B', 'G', 'R', '3'), fourcc('B', 'G', 'R', '4') };
    y_bgr.function = y_to_bgr;

    Converter yuy2_bgr;
    yuy2_bgr.source_fmts = { fourcc('Y', 'U', 'Y', '2'), fourcc('Y', 'U', 'Y', 'V') };
    yuy2_bgr.target_fmts = { fourcc('B', 'G', 'R', '3'), fourcc('B', 'G', 'R', '4') };
    yuy2_bgr.function = yuy2_to_bgr;

    register_converter(bgr_rgb, 0 /* prio */);
    register_converter(rgb_bgr, 0 /* prio */);
    register_converter(rgb_y, 0 /* prio */);
    register_converter(y_bgr, 0 /* prio */);
    register_converter(yuy2_bgr, 0 /* prio */);
}

bool ImageBufferConverter::convert(const Source &src, Target &dst) {
    Converter conv = {};
    int prio = -1;

    for (const auto &candidate : m_converters) {
        // check source and target format support
        if (std::find(candidate.second.source_fmts.begin(), candidate.second.source_fmts.end(),
                      src.fourcc) != candidate.second.source_fmts.end() &&
            std::find(candidate.second.target_fmts.begin(), candidate.second.target_fmts.end(),
                      dst.fourcc) != candidate.second.target_fmts.end()) {
            if (!conv.function || prio < candidate.first) {
                conv = candidate.second;
            }
        }
    }

    if (!conv.function) {
        return false;
    }

    return conv.function(src, dst);
}

size_t ImageBufferConverter::probe(const Source &src, Target &dst) {
    size_t dst_size;
    size_t dst_pixel_size;
    size_t dst_padding;
    size_t dst_stride;

    switch (dst.fourcc) {
    case fourcc('B', 'G', 'R', '3'):
        dst_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('B', 'G', 'R', '4'):
        dst_pixel_size = 4; /* 32 bpp */
        break;
    case fourcc('R', 'G', 'B', '3'):
        dst_pixel_size = 3; /* 24 bpp */
        break;
    case fourcc('R', 'G', 'B', '4'):
        dst_pixel_size = 4; /* 32 bpp */
        break;
    case fourcc('G', 'R', 'E', 'Y'):
        dst_pixel_size = 1; /* 8 bpp */
        break;
    case fourcc('Y', '1', '6', ' '):
        dst_pixel_size = 2; /* 16 bpp */
        break;
    default:
        return false;
    }

    dst_padding = (dst.alignment - ((src.width * dst_pixel_size) % dst.alignment)) % dst.alignment;
    dst_stride = src.width * dst_pixel_size + dst_padding;
    dst_size = src.height * dst_stride;

    return dst_size;
}
