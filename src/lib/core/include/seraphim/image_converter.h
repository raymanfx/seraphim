/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_IMAGE_CONVERTER_H
#define SPH_CORE_IMAGE_CONVERTER_H

#include <functional>
#include <vector>

#include "image.h"
#include "pixelformat.h"

namespace sph {

/**
 * @brief Image converter singleton facility.
 *
 * The central instance in the platform for image buffer conversions.
 * Some converters are implemented by default, but you should register your own (possibly more
 * efficient ones) here to everyone can profit.
 */
class ImageConverter {
public:
    /**
     * @brief Singleton class instance.
     * @return The single, static instance of this class.
     */
    static ImageConverter &Instance() {
        // Guaranteed to be destroyed, instantiated on first use.
        static ImageConverter instance;
        return instance;
    }

    // Remove copy and assignment constructors.
    ImageConverter(ImageConverter const &) = delete;
    void operator=(ImageConverter const &) = delete;

    /**
     * @brief Convert arbitrary pixel data into a supported format.
     */
    typedef std::function<bool(const Image &src, CoreImage &dst, const Pixelformat &fmt)>
        ConverterFunction;

    /**
     * @brief Converter entity that converts between image buffer formats.
     */
    struct Converter {
        /// source color space
        std::vector<Pixelformat::Color> src;
        /// target color space
        std::vector<Pixelformat::Color> dst;
        /// converter function that transforms images
        ConverterFunction function;
    };

    /**
     * @brief Register a new format converter. Converters added later take precedence over others,
     *        if their priority is higher or equal.
     * @param converter The converter that performs the actual pixel conversion.
     */
    void register_converter(const struct Converter &converter, int prio = 0) {
        m_converters.push_back(std::make_pair(prio, converter));
    }

    /**
     * @brief Create a new image buffer.
     * @param src Source image.
     * @param dst Target image.
     * @param fmt Target format.
     * @return True on success, false otherwise.
     */
    bool convert(const Image &src, CoreImage &dst, const Pixelformat &fmt);

private:
    ImageConverter();

    /// available image buffer converters
    std::vector<std::pair<int, Converter>> m_converters;
};

} // namespace sph

#endif // SPH_CORE_IMAGE_CONVERTER_H
