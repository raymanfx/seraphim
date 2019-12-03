/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <csignal>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <optparse.h>
#include <seraphim/polygon.h>
#include <seraphim/gui.h>
#include <seraphim/iop/opencv/mat.h>
#include <seraphim/object/dnn_detector.h>

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
    float confidence_threshold = 0;
    std::string model_path;
    std::string model_config_path;
    sph::object::DNNDetector detector;
    std::vector<sph::object::Detector::Prediction> predictions;
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

    sph::cmd::Option thresholdOpt;
    inputOpt.name = "threshold";
    inputOpt.shortname = "t";
    inputOpt.description = "Confidence threshold";
    inputOpt.arg = true;
    optparse.add(inputOpt, [&](const std::string &val) {
        confidence_threshold = std::stof(val);
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

    sph::cmd::Option configOpt;
    inputOpt.name = "config";
    inputOpt.shortname = "c";
    inputOpt.description = "File path";
    inputOpt.arg = true;
    inputOpt.required = true;
    optparse.add(inputOpt, [&](const std::string &val) {
        model_config_path = val;
    });

    sph::cmd::Option helpOpt;
    helpOpt.name = "help";
    helpOpt.shortname = "h";
    helpOpt.description = "Show help";
    optparse.add(helpOpt, [&](const std::string&) {
        std::cout << "dnn_face_detector [args]" << std::endl << std::endl;
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

    if (model_path.empty() || model_config_path.empty()) {
        std::cout << "[ERROR] Missing model or config file" << std::endl;
        return 1;
    }

    cv::VideoCapture cap;
    if (!cap.open(camera_index, cv::CAP_V4L2)) {
        if (!cap.open(camera_index)) {
            std::cout << "[ERROR] Failed to open camera: " << camera_index << std::endl;
            return 1;
        }
    }

    if (!detector.read_net(model_path, model_config_path)) {
        std::cout << "[ERROR Failed to read net" << std::endl;
        return 1;
    }
    detector.set_target(sph::Computable::Target::CPU);

    auto viewer = sph::gui::WindowFactory::create("DNN Detector");

    // parameters from:
    // https://becominghuman.ai/face-detection-with-opencv-and-deep-learning-90b84735f421
    sph::object::DNNDetector::BlobParameters params = {};
    params.scalefactor = 1.0;
    params.size = cv::Size(300, 300);
    params.mean = cv::Scalar(103.93, 116.77, 123.68);
    detector.set_blob_parameters(params);

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

        predictions.clear();
        image = sph::iop::cv::to_image(frame);
        if (image.empty()) {
            std::cout << "[ERROR] Failed to convert Mat to Image" << std::endl;
            continue;
        }

        detector.predict(image, predictions);
        process_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::high_resolution_clock::now() - t_frame_captured)
                           .count();

        if (predictions.size() > 0) {
            for (auto &pred : predictions) {
                if (pred.confidence < confidence_threshold) {
                    continue;
                }

                cv::rectangle(frame,
                              cv::Rect(pred.poly.tl().x, pred.poly.tl().y, pred.poly.width(),
                                       pred.poly.height()),
                              cv::Scalar(0, 255, 0), 2);
                std::string label = cv::format("%.2f", pred.confidence);
                int baseLine;
                int top = pred.poly.tl().y;
                int left = pred.poly.tl().x;
                cv::Size labelSize =
                    cv::getTextSize(label, cv::FONT_HERSHEY_DUPLEX, 1.0, 1, &baseLine);
                top = cv::max(top, labelSize.height);
                cv::rectangle(frame, cv::Point(left, top),
                              cv::Point(left + labelSize.width, top + labelSize.height + baseLine),
                              cv::Scalar(0, 255, 0), cv::FILLED);
                cv::putText(frame, label, cv::Point(left, top + labelSize.height),
                            cv::FONT_HERSHEY_DUPLEX, 1.0, cv::Scalar::all(255));
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
