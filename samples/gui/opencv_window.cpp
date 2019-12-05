/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <chrono>
#include <csignal>
#include <iostream>
#include <opencv2/videoio.hpp>
#include <optparse.h>
#include <seraphim/image.h>
#include <seraphim/gui.h>
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
    int camera_index = 0;
    sph::CoreImage image;
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
    inputOpt.description = "Camera index";
    inputOpt.arg = true;
    optparse.add(inputOpt, [&](const std::string &val) {
        camera_index = std::stoi(val);
    });

    sph::cmd::Option helpOpt;
    helpOpt.name = "help";
    helpOpt.shortname = "h";
    helpOpt.description = "Show help";
    optparse.add(helpOpt, [&](const std::string&) {
        std::cout << "opencv_window [args]" << std::endl << std::endl;
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

    cv::VideoCapture cap;
    if (!cap.open(camera_index)) {
        if (!cap.open(camera_index)) {
            std::cout << "[ERROR] Failed to open camera: " << camera_index << std::endl;
            return 1;
        }
    }

    auto window = sph::gui::WindowFactory::create("OpenCV Window", sph::gui::WindowFactory::Impl::OPENCV);

    auto event_cb = [&](sph::gui::Window::Event event) {
        if (event & sph::gui::Window::Event::CLOSE) {
            main_loop = 0;
        }
    };
    window->subscribe(event_cb, sph::gui::Window::Event::CLOSE);

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

        image = sph::iop::cv::to_image(frame);
        if (image.empty()) {
            std::cout << "[ERROR] Failed to convert Mat to Image" << std::endl;
            continue;
        }

        window->show(image);
        process_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::high_resolution_clock::now() - t_frame_captured)
                           .count();

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
    }
}
