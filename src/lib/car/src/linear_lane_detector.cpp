/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include "seraphim/car/linear_lane_detector.h"

using namespace sph::car;

LinearLaneDetector::LinearLaneDetector() {
    m_params = {};
    m_params.roi.push_back(cv::Point(210, 720 /* img.rows() */));
    m_params.roi.push_back(cv::Point(550, 450));
    m_params.roi.push_back(cv::Point(717, 450));
    m_params.roi.push_back(cv::Point(1280 /* img.cols() */, 720 /* img.rows() */));

    m_params.canny_low_thresh = 50;
    m_params.canny_ratio = 3;
    m_params.canny_kernel_size = 3;
    m_params.canny_use_l2_dist = false;

    m_params.hough_rho = 1;
    m_params.hough_theta = CV_PI / 180;
    m_params.hough_thresh = 20;
    m_params.hough_min_line_len = 20;
    m_params.hough_max_line_len = 30;

    m_target = TARGET_CPU;
}

LinearLaneDetector::~LinearLaneDetector() {
    // dummy
}

void LinearLaneDetector::preprocess(cv::InputArray img, cv::OutputArray out) {
    // convert image to grayscale
    if (img.channels() > 1) {
        cv::cvtColor(img, out, cv::COLOR_BGR2GRAY);
    } else {
        img.copyTo(out);
    }
}

void LinearLaneDetector::filter_edges(cv::InputArray img, cv::OutputArray out) {
    cv::Canny(img, out, m_params.canny_low_thresh, m_params.canny_low_thresh * m_params.canny_ratio,
              m_params.canny_kernel_size, m_params.canny_use_l2_dist);
}

void LinearLaneDetector::apply_mask(cv::InputArray img, cv::OutputArray out,
                                    const std::vector<cv::Point> &poly) {
    cv::Mat mask = cv::Mat::zeros(img.size(), img.type());

    if (mask.empty()) {
        return;
    }

    // Create a binary polygon mask
    cv::fillConvexPoly(mask, poly.data(), static_cast<int>(poly.size()), cv::Scalar(255, 0, 0));

    // Multiply the edges image and the mask to get the output
    cv::bitwise_and(img, mask, out);
}

void LinearLaneDetector::detect_lines(cv::InputArray img, cv::OutputArray &lines) {
    cv::HoughLinesP(img, lines, m_params.hough_rho, m_params.hough_theta, m_params.hough_thresh,
                    m_params.hough_min_line_len, m_params.hough_max_line_len);
}

bool LinearLaneDetector::set_target(const target_t &target) {
    std::unique_lock<std::mutex> lock(m_target_mutex);

    switch (target) {
    case TARGET_CPU:
    case TARGET_OPENCL:
        m_target = target;
        break;
    default:
        /* unsupported */
        return false;
    }

    return true;
}

bool LinearLaneDetector::detect(cv::InputArray img, std::vector<Lane> &lanes) {
    std::vector<cv::Vec4i> segments;
    std::vector<cv::Point> left_points;
    std::vector<cv::Point> right_points;
    cv::Vec4d left_line_params;
    cv::Vec4d right_line_params;
    double slope_thresh = 0.3;
    std::unique_lock<std::mutex> lock(m_target_mutex);

    switch (m_target) {
    case TARGET_CPU:
        preprocess(img, m_mat_buf);
        filter_edges(m_mat_buf, m_mat_buf);
        apply_mask(m_mat_buf, m_mat_buf, m_params.roi);
        detect_lines(m_mat_buf, segments);
        break;
    case TARGET_OPENCL:
        preprocess(img, m_umat_buf);
        filter_edges(m_umat_buf, m_umat_buf);
        apply_mask(m_umat_buf, m_umat_buf, m_params.roi);
        detect_lines(m_umat_buf, segments);
        break;
    default:
        /* unsupported */
        return false;
    }

    if (segments.empty()) {
        return false;
    }

    // filter left and right segments
    for (const cv::Vec4i &seg : segments) {
        cv::Point p1 = cv::Point(seg[0], seg[1]);
        cv::Point p2 = cv::Point(seg[2], seg[3]);

        // algebra: slope = (y2-y1)/(x2-x1)
        double slope = (p2.y - p1.y) * 1.0 / (p2.x - p1.x);
        if (std::abs(slope) < slope_thresh) {
            continue;
        }

        if (slope > 0.0 && p1.x > img.cols() / 2 && p2.x > img.cols() / 2) {
            // positive slope: right line
            left_points.push_back(p1);
            left_points.push_back(p2);
        } else if (p1.x < img.cols() / 2 && p2.x < img.cols() / 2) {
            // negative slope: left line
            right_points.push_back(p1);
            right_points.push_back(p2);
        }
    }

    if (left_points.empty() || right_points.empty()) {
        return false;
    }

    // fit lines through the filtered segments using method of least squares (cv::DIST_L2)
    // according to OpenCV docs, 0.01 is a good value for reps and aeps..
    cv::fitLine(left_points, left_line_params, cv::DIST_L2 /* distType */, 0 /* param */,
                0.01 /* reps */, 0.01 /* aeps */);
    cv::fitLine(right_points, right_line_params, cv::DIST_L2 /* distType */, 0 /* param */,
                0.01 /* reps */, 0.01 /* aeps */);

    // line params format: ([0], [1]) is a normalized vector colinear to the line,
    // ([2], [3]) is a point on the line

    Lane lane;
    // for the two bottom points of the lane (left and right lines), coordinates are determined by
    // the line parameters:
    // find the point on the line that intersects with the bottom line of the image going
    // from (0, rows) to (cols, rows)

    // algebra: y = m * x + b
    // x = (y - b) / m
    lane.bottomLeft.y = img.rows();
    lane.bottomLeft.x = static_cast<int>((lane.bottomLeft.y - left_line_params[3]) /
                                             (left_line_params[1] / left_line_params[0]) +
                                         left_line_params[2]);
    lane.bottomRight.y = img.rows();
    lane.bottomRight.x = static_cast<int>((lane.bottomRight.y - right_line_params[3]) /
                                              (right_line_params[1] / right_line_params[0]) +
                                          right_line_params[2]);

    // the two top points are determined from the top line segment points with respect to their
    // y coordinate
    // their x coordinates are then calculated by applying the line equation again

    // take the first point in the vector as starting point
    lane.topLeft.y = left_points[0].y;
    for (const cv::Point &p : left_points) {
        // use "<" here because the coordinate origin is in the top left corner for OpenCV
        // so if we are looking for the topmost point, it has to have the smallest y coordinate
        if (p.y < lane.topLeft.y) {
            lane.topLeft.y = p.y;
        }
    }
    lane.topRight.y = right_points[0].y;
    for (const cv::Point &p : right_points) {
        // use "<" here because the coordinate origin is in the top left corner for OpenCV
        // so if we are looking for the topmost point, it has to have the smallest y coordinate
        if (p.y < lane.topRight.y) {
            lane.topRight.y = p.y;
        }
    }

    // make sure we use the same y coordinate for the left and right line of the lane
    lane.topLeft.y = std::max(lane.topLeft.y, lane.topRight.y);
    lane.topRight.y = lane.topLeft.y;

    lane.topLeft.x = static_cast<int>((lane.topLeft.y - left_line_params[3]) /
                                          (left_line_params[1] / left_line_params[0]) +
                                      left_line_params[2]);
    lane.topRight.x = static_cast<int>((lane.topRight.y - right_line_params[3]) /
                                           (right_line_params[1] / right_line_params[0]) +
                                       right_line_params[2]);

    lanes.push_back(lane);

    return true;
}
