/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <csignal>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <optparse.hpp>
#include <seraphim/polygon.hpp>
#include <seraphim/face/lbf_facemark_detector.hpp>
#include <seraphim/face/lbp_face_detector.hpp>
#include <seraphim/gui.hpp>
#include <seraphim/iop/opencv/mat.hpp>

using namespace sph::face;

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
    std::string model_path;
    std::string cascade_path;
    std::shared_ptr<LBPFaceDetector> face_detector =
        std::shared_ptr<LBPFaceDetector>(new LBPFaceDetector());
    LBFFacemarkDetector facemark_detector;
    std::vector<sph::Polygon<int>> faces;
    std::vector<FacemarkDetector::Facemarks> facemarks;
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

    sph::cmd::Option modelOpt;
    inputOpt.name = "model";
    inputOpt.shortname = "m";
    inputOpt.description = "File path";
    inputOpt.arg = true;
    inputOpt.required = true;
    optparse.add(inputOpt, [&](const std::string &val) {
        model_path = val;
    });

    sph::cmd::Option cascadeOpt;
    inputOpt.name = "cascade";
    inputOpt.shortname = "c";
    inputOpt.description = "File path";
    inputOpt.arg = true;
    inputOpt.required = true;
    optparse.add(inputOpt, [&](const std::string &val) {
        cascade_path = val;
    });

    sph::cmd::Option helpOpt;
    helpOpt.name = "help";
    helpOpt.shortname = "h";
    helpOpt.description = "Show help";
    optparse.add(helpOpt, [&](const std::string&) {
        std::cout << "lbf_facemark_detector [args]" << std::endl << std::endl;
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

    if (model_path.empty() || cascade_path.empty()) {
        std::cout << "[ERROR] Missing model or cascade path" << std::endl;
        return 1;
    }

    cv::VideoCapture cap;
    if (!cap.open(camera_index, cv::CAP_V4L2)) {
        if (!cap.open(camera_index)) {
            std::cout << "[ERROR] Failed to open camera: " << camera_index << std::endl;
            return 1;
        }
    }

    if (!face_detector->load_face_cascade(cascade_path)) {
        std::cout << "[ERROR Failed to read cascade" << std::endl;
        return 1;
    }
    face_detector->set_target(sph::Computable::Target::CPU);

    if (!facemark_detector.load_facemark_model(model_path)) {
        std::cout << "[ERROR Failed to read model" << std::endl;
        return 1;
    }
    facemark_detector.set_target(sph::Computable::Target::CPU);

    auto viewer = sph::gui::WindowFactory::create("LBF Facemark Detector");

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

        faces.clear();
        facemarks.clear();
        image = sph::iop::cv::to_image(frame);
        if (image.empty()) {
            std::cout << "[ERROR] Failed to convert Mat to Image" << std::endl;
            continue;
        }

        face_detector->detect(image, faces);
        if (faces.size() > 0) {
            facemark_detector.detect(image, faces, facemarks);
        }
        process_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::high_resolution_clock::now() - t_frame_captured)
                           .count();

        for (const auto &face : facemarks) {
            for (const auto &landmark : face.landmarks) {
                for (const auto &point : landmark.second) {
                    cv::circle(frame, cv::Point(point.x, point.y), 1, cv::Scalar(0, 255, 0), 2);
                }
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

        viewer->show(image);
    }
}
