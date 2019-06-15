/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CAR_LANE_DETECTOR_EZ_H
#define SPH_CAR_LANE_DETECTOR_EZ_H

#include "lane_detector.h"
#include <mutex>
#include <opencv2/opencv.hpp>
#include <seraphim/core/computable.h>
#include <vector>

namespace sph {
namespace car {

/**
 * @brief Simple, forward single-lane detector.
 *
 * A very simple image processing pipeline that extracts a single lane from an image.
 * Turn prediction is also included.
 *
 * The pipeline looks like this:
 *   1) preprocessing (grayscale conversion)
 *   2) canny edge detection
 *   3) region masking to filter region of interest
 *   4) hough line transformation
 *   5) linear regression for left and right segments
 */
class EZLaneDetector : public ILaneDetector, public sph::core::IComputable {
public:
    EZLaneDetector();
    ~EZLaneDetector() override;

    /**
     * @brief Parameters used by algorithms in the image processing pipeline.
     */
    struct Parameters {
        // region of interest masking
        std::vector<cv::Point> roi;

        // canny edge detection params
        double canny_low_thresh;
        double canny_ratio;
        int canny_kernel_size;
        bool canny_use_l2_dist;

        // hough line transform params
        double hough_rho;
        double hough_theta;
        int hough_thresh;
        double hough_min_line_len;
        double hough_max_line_len;
    };

    /**
     * @brief Preprocess an image by applying Gaussian blur and converting it to grayscale.
     * @param img The image to operate on.
     * @param out The processed image.
     */
    void preprocess(cv::InputArray img, cv::OutputArray out);

    /**
     * @brief Filter edges by converting to a binary image and applying a threshold.
     * @param img The image to operate on.
     * @param out The computed image.
     */
    void filter_edges(cv::InputArray img, cv::OutputArray out);

    /**
     * @brief Apply a polynomial mask to an image, setting all outside pixel values to 0.
     * @param img The image to operate on.
     * @param out The computed image.
     * @param mask Vector of points forming a single polygon.
     */
    void apply_mask(cv::InputArray img, cv::OutputArray out, const std::vector<cv::Point> &mask);

    /**
     * @brief Apply hough line transform on a 8 bit single channel image.
     * @param img The image to operate on.
     * @param lines The computed line segments.
     */
    void detect_lines(cv::InputArray img, cv::OutputArray lines);

    /**
     * @brief Set parameters for various algorithms that are used by this class.
     * @param params Parameters for canny edge detection and hough line transform.
     */
    void set_parameters(const Parameters &params) { m_params = params; }

    bool set_target(const target_t &target) override;

    bool detect(cv::InputArray img, std::vector<Lane> &lanes) override;

private:
    /// parameters used by the image processing pipeline
    Parameters m_params;

    cv::Mat m_mat_buf;
    cv::UMat m_umat_buf;
    target_t m_target;
    std::mutex m_target_mutex;
};

} // namespace car
} // namespace sph

#endif // SPH_CAR_LANE_DETECTOR_EZ_H
