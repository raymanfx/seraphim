/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_IMAGE_BUFFER_CONVERTER_H
#define SPH_CORE_IMAGE_BUFFER_CONVERTER_H

#include <functional>
#include <vector>

#include "fourcc.h"

namespace sph {
namespace core {

/**
 * @brief Image converter singleton facility.
 *
 * The central instance in the platform for image buffer conversions.
 * Some converters are implemented by default, but you should register your own (possibly more
 * efficient ones) here to everyone can profit.
 */
class ImageBufferConverter {
public:
    /**
     * @brief Singleton class instance.
     * @return The single, static instance of this class.
     */
    static ImageBufferConverter &Instance() {
        // Guaranteed to be destroyed, instantiated on first use.
        static ImageBufferConverter instance;
        return instance;
    }

    // Remove copy and assignment constructors.
    ImageBufferConverter(ImageBufferConverter const &) = delete;
    void operator=(ImageBufferConverter const &) = delete;

    /**
     * @brief Source buffer description.
     */
    struct SourceFormat {
        /// width in pixels
        uint32_t width;
        /// height in pixels
        uint32_t height;
        /// pixel row padding
        uint32_t padding;
        /// pixelformat
        uint32_t fourcc;
    };

    /**
     * @brief Target buffer description.
     */
    struct TargetFormat {
        /// padding for each pixel row (padding = stride - width)
        uint32_t padding;
        /// pixelformat
        uint32_t fourcc;
    };

    /**
     * @brief Convert arbitrary pixel data into a supported format.
     */
    typedef std::function<size_t(unsigned char **src, const SourceFormat &src_fmt,
                                 unsigned char **dst, const TargetFormat &dst_fmt)>
        ConverterFunction;

    /**
     * @brief Converter entity that converts between image buffer formats.
     */
    struct Converter {
        /// source pixelformats as four character code
        std::vector<uint32_t> source_fmts;
        /// target pixelformats as four character code
        std::vector<uint32_t> target_fmts;
        /// converter function that transforms pixel buffers
        ConverterFunction function;
    };

    /**
     * @brief Register a new format converter. Converters added later take precedence over others,
     *        if their priority is higher or equal.
     * @param converter The converter that performs the actual pixel conversion.
     */
    void register_converter(const struct Converter &converter, const int &prio = 0) {
        m_converters.push_back(std::make_pair(prio, converter));
    }

    /**
     * @brief Create a new image buffer.
     * @param src Pixelbuffer source.
     * @param src_fmt Pixelbuffer source format.
     * @param dst Target buffer.
     * @param dst_fmt Target buffer format.
     * @return The size of the converted buffer.
     */
    size_t convert(unsigned char **src, const SourceFormat &src_fmt, unsigned char **dst,
                   const TargetFormat &dst_fmt);

private:
    ImageBufferConverter();

    /// available image buffer converters
    std::vector<std::pair<int, Converter>> m_converters;
};

} // namespace core
} // namespace sph

#endif // SPH_CORE_IMAGE_BUFFER_CONVERTER_H
