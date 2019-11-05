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

#include "pixelformat.h"

namespace sph {

class Image;
class BufferedImage;

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
     * @brief Source buffer description.
     */
    struct Source {
        /// image representation
        const Image *img = nullptr;
        /// pixelformat
        uint32_t fourcc = 0;
    };

    /**
     * @brief Target buffer description.
     */
    struct Target {
        /// image representation
        BufferedImage *img = nullptr;
        /// pixelformat
        sph::Pixelformat::Enum fmt = sph::Pixelformat::Enum::UNKNOWN;
    };

    /**
     * @brief Convert arbitrary pixel data into a supported format.
     */
    typedef std::function<bool(const Source &src, Target &dst)> ConverterFunction;

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
    void register_converter(const struct Converter &converter, int prio = 0) {
        m_converters.push_back(std::make_pair(prio, converter));
    }

    /**
     * @brief Create a new image buffer.
     * @param src Source buffer description.
     * @param dst Target buffer description.
     * @return True on success, false otherwise.
     */
    bool convert(const Source &src, Target &dst);

    /**
     * @brief Create a new image buffer.
     * @param src Source image.
     * @param dst Target image.
     * @param fmt Target image format.
     * @return True on success, false otherwise.
     */
    bool convert(const Image &src, BufferedImage &dst, sph::Pixelformat::Enum fmt);

private:
    ImageConverter();

    /// available image buffer converters
    std::vector<std::pair<int, Converter>> m_converters;
};

} // namespace sph

#endif // SPH_CORE_IMAGE_CONVERTER_H
