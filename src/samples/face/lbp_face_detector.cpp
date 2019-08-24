/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <csignal>
#include <getopt.h>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <seraphim/core/polygon.h>
#include <seraphim/face/lbp_face_detector.h>
#include <seraphim/iop/opencv/mat.h>

using namespace sph::face;

static bool main_loop = true;

static struct option long_opts[] = { { "camera", required_argument, 0, 'i' },
                                     { "model", required_argument, 0, 'm' },
                                     { "help", no_argument, 0, 'h' },
                                     { 0, 0, 0, 0 } };

static char const *long_opts_desc[] = { "Camera index", "Pretrained cascade classifier",
                                        "Show help" };

static void print_usage(int print_description) {
    unsigned int max_name_len = 0, max_desc_len = 0;

    for (size_t i = 0; i < sizeof(long_opts) / sizeof(long_opts[0]) - 1; i++) {
        if (max_name_len < strlen(long_opts[i].name)) {
            max_name_len = static_cast<unsigned int>(strlen(long_opts[i].name));
        }
    }

    for (size_t i = 0; i < sizeof(long_opts_desc) / sizeof(long_opts_desc[0]); i++) {
        if (max_desc_len < strlen(long_opts_desc[i])) {
            max_desc_len = static_cast<unsigned int>(strlen(long_opts_desc[i]));
        }
    }

    printf("%s\n\n", "lbp_face_detector [flags]");
    for (size_t i = 0; i < sizeof(long_opts) / sizeof(long_opts[0]) - 1; i++) {
        const struct option opt = long_opts[i];

        printf("    -%c    --%-*s", opt.val, max_name_len, opt.name);
        switch (opt.has_arg) {
        case no_argument:
            printf("    %-20s", "no_argument");
            break;
        case required_argument:
            printf("    %-20s", "required_argument");
            break;
        case optional_argument:
            printf("    %-20s", "optional_argument");
            break;
        default:
            break;
        }

        if (print_description)
            printf("    %-*s", max_desc_len, long_opts_desc[i]);

        printf("\n");
    }
}

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
    std::string cascade_path;
    ::LBPFaceDetector detector;
    std::vector<sph::core::Polygon<>> faces;
    sph::core::Image image;
    cv::Mat frame;
    std::chrono::high_resolution_clock::time_point t_loop_start;
    std::chrono::high_resolution_clock::time_point t_frame_captured;
    long frame_time;
    long process_time;
    long fps;
    long elapsed = 0;

    // register signal handler
    signal(SIGINT, signal_handler);

    int opt = 0;
    int long_index = 0;
    while ((opt = getopt_long(argc, argv, "i:m:h", long_opts, &long_index)) != -1) {
        switch (opt) {
        case 'i':
            camera_index = std::stoi(optarg);
            break;
        case 'm':
            cascade_path = std::string(optarg);
            break;
        case 'h':
            print_usage(1);
            return 0;
        default:
            print_usage(0);
            return 1;
        }
    }

    if (cascade_path.empty()) {
        std::cout << "[ERROR] Missing cascade classifier path" << std::endl;
        return 1;
    }

    cv::VideoCapture cap;
    if (!cap.open(camera_index, cv::CAP_V4L2)) {
        if (!cap.open(camera_index)) {
            std::cout << "[ERROR] Failed to open camera: " << camera_index << std::endl;
            return 1;
        }
    }

    if (!detector.load_face_cascade(cascade_path)) {
        std::cout << "[ERROR Failed to read cascade" << std::endl;
        return 1;
    }
    detector.set_target(sph::core::IComputable::TARGET_CPU);

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
        image = sph::iop::cv::MatFacility::to_image(frame);
        if (image.empty()) {
            std::cout << "[ERROR] Failed to convert Mat to Image" << std::endl;
            continue;
        }

        detector.detect_faces(image, faces);
        process_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::high_resolution_clock::now() - t_frame_captured)
                           .count();

        for (const auto &poly : faces) {
            cv::rectangle(frame, cv::Rect(poly.tl().x, poly.tl().y, poly.width(), poly.height()),
                          cv::Scalar(0, 255, 0), 2);
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

        cv::imshow("LBP Face Detector", frame);
        cv::waitKey(1);
    }
}
