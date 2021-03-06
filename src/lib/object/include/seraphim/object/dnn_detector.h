/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_OBJECT_DETECTOR_DNN_H
#define SPH_OBJECT_DETECTOR_DNN_H

#include "detector.h"
#include <mutex>
#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>
#include <seraphim/computable.h>
#include <vector>

namespace sph {
namespace object {

/**
 * @brief Deep neural network object detector.
 *
 * Using the OpenCV dnn backend under the hood, this class allows you to load
 * models from Caffe, TensorFlow and many more frameworks.
 * The computation target can be set to CPU, OPENCL, VPU and more.
 */
class DNNDetector : public Detector, public sph::Computable {
public:
    DNNDetector() = default;

    /**
     * @brief Parameters used for preparing input images.
     */
    struct BlobParameters {
        /// input image scaling
        double scalefactor = 1.0;
        /// input image resizing
        cv::Size size = cv::Size(300, 300);
        /// mean substraction
        cv::Scalar mean = cv::Scalar();
        /// whether to swap R/B channels of the input image
        bool swap_rb = false;
        /// input image cropping
        bool crop = false;
    };

    void set_blob_parameters(const struct BlobParameters &params) { m_blob_params = params; }

    /**
     * @brief Read a network from a model and a config.
     * @param model Binary file containing trained weights.
     * @param config Text file containing network configuration.
     * @param framework Explicit framework name, optional.
     * @return true on success, false otherwise.
     */
    bool read_net(const std::string &model, const std::string &config,
                  const std::string &framework = "");

    /**
     * @brief Backport of OpenCV 3.4.4 getUnconnectedOutLayersNames().
     * @return Array of out layer names.
     */
    std::vector<std::string> get_unconnected_out_layer_names();

    /**
     * @brief Set the preferred conputation backend.
     * @param id Backend id, e.g. DNN_BACKEND_OPENCV.
     */
    void set_preferrable_backend(int id) { m_net.setPreferableBackend(id); }

    /**
     * @brief Set the preferred conputation target.
     * @param id Target id, e.g. DNN_TARGET_CPU.
     */
    void set_preferrable_target(int id) { m_net.setPreferableTarget(id); }

    bool set_target(Target target) override;

    bool predict(const sph::Image &img, std::vector<Prediction> &preds) override;

private:
    /// Deep neural network
    cv::dnn::Net m_net;

    /// Parameters for cv::blobFromImage
    struct BlobParameters m_blob_params;

    /// Layer names have to be refreshed after @ref read_net was called.
    bool m_refresh_layer_names = false;

    std::mutex m_target_mutex;
};

} // namespace object
} // namespace sph

#endif // SPH_OBJECT_DETECTOR_DNN_H
