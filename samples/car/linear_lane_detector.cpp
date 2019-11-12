/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <csignal>
#include <opencv2/videoio.hpp>
#include <optparse.h>
#include <seraphim/car/linear_lane_detector.h>
#include <seraphim/polygon.h>
#include <seraphim/gui/gl_window.h>
#include <seraphim/iop/opencv/mat.h>

static bool main_loop = true;

void signal_handler(int signal) {
    switch (signal) {
    case SIGINT:
        std::cout << "[SIGNAL] SIGINT: Terminating application" << std::endl;
        main_loop = false;
        break;
    default:
        std::cout << "[SIGNAL] unknown: " << signal << std::endl;
        break;
    }
}

int main(int argc, char **argv) {
    std::string file_path;
    sph::car::LinearLaneDetector lane_detector;
    std::vector<sph::Polygon<int>> lanes;
    sph::VolatileImage image;
    cv::Mat frame;
    std::chrono::high_resolution_clock::time_point t_loop_start;
    std::chrono::high_resolution_clock::time_point t_frame_captured;
    long frame_time;
    long process_time;
    long fps;
    long elapsed = 0;

    // register signal handler
    signal(SIGINT, signal_handler);

    // build args
    sph::cmd::OptionParser optparse;

    sph::cmd::Option inputOpt;
    inputOpt.name = "input";
    inputOpt.shortname = "i";
    inputOpt.description = "File path";
    inputOpt.arg = true;
    optparse.add(inputOpt, [&](const std::string &val) {
        file_path = val;
    });

    sph::cmd::Option helpOpt;
    helpOpt.name = "help";
    helpOpt.shortname = "h";
    helpOpt.description = "Show help";
    optparse.add(helpOpt, [&](const std::string&) {
        std::cout << "linear_lane_detector [args]" << std::endl << std::endl;
        for (const auto &str : optparse.help(true)) {
            std::cout << str << std::endl;
        }
        exit(0);
    });

    try {
        optparse.parse(argc, argv);
    } catch (const std::exception &e) {
        std::cout << "[ERROR] " << e.what() << std::endl;
        return 0;
    }

    if (file_path.empty()) {
        std::cout << "[ERROR] Missing input file" << std::endl;
        return 1;
    }

    cv::VideoCapture cap;
    if (!cap.open(file_path)) {
        std::cout << "[ERROR] Failed to open file: " << file_path << std::endl;
        return 1;
    }

    if (!cap.read(frame)) {
        std::cout << "[ERROR] Failed to read frame" << std::endl;
        return 1;
    }

    sph::gui::GLWindow viewer("Linear Lane Detector");

    // initialize parameters for lane detector
    sph::car::LinearLaneDetector::Parameters params = {};
    params.canny_low_thresh = 50;
    params.canny_ratio = 3;
    params.canny_kernel_size = 3;
    params.canny_use_l2_dist = false;
    params.hough_rho = 1;
    params.hough_theta = CV_PI / 180;
    params.hough_thresh = 20;
    params.hough_min_line_len = 20;
    params.hough_max_line_len = 30;
    lane_detector.set_parameters(params);

    // tune the ROI according to your input video
    // in this case, use a 4-point polygon shape to match the "project_video.mp4"
    // clip of the udacity course at https://github.com/udacity/CarND-Vehicle-Detection
    sph::Polygon<int> roi;
    // bottom left
    roi.add_point({ 210, frame.rows });
    // top left
    roi.add_point({ 550, 450 });
    // top right
    roi.add_point({ 717, 450 });
    // bottom right
    roi.add_point({ frame.cols, frame.rows });
    lane_detector.set_roi(roi);

    while (main_loop) {
        t_loop_start = std::chrono::high_resolution_clock::now();

        if (!cap.read(frame)) {
            std::cout << "[ERROR] Failed to read frame" << std::endl;
            break;
        }

        t_frame_captured = std::chrono::high_resolution_clock::now();
        frame_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::high_resolution_clock::now() - t_loop_start)
                         .count();

        lanes.clear();
        image = sph::iop::cv::MatFacility::to_image(frame);
        if (image.empty()) {
            std::cout << "[ERROR] Failed to convert Mat to Image" << std::endl;
            continue;
        }

        lane_detector.detect(image, lanes);
        process_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::high_resolution_clock::now() - t_frame_captured)
                           .count();

        if (lanes.size() > 0) {
            for (auto &lane : lanes) {
                cv::line(frame, cv::Point(lane.points()[0].x, lane.points()[0].y),
                         (cv::Point(lane.points()[1].x, lane.points()[1].y)), cv::Scalar(0, 0, 255),
                         3);
                cv::line(frame, cv::Point(lane.points()[2].x, lane.points()[2].y),
                         (cv::Point(lane.points()[3].x, lane.points()[3].y)), cv::Scalar(0, 0, 255),
                         3);
            }
        }

        fps = 1000 / std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::high_resolution_clock::now() - t_loop_start)
                         .count();

        elapsed += std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::high_resolution_clock::now() - t_loop_start)
                       .count();
        if (elapsed >= 500) {
            std::cout << "\r"
                      << "[INFO] frame time: " << frame_time << " ms"
                      << ", process time: " << process_time << " ms"
                      << ", fps: " << fps << std::flush;
            elapsed = 0;
        }

        viewer.show(image);
    }
}
