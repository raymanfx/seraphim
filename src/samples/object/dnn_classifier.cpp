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
#include <seraphim/core/image_utils_opencv.h>
#include <seraphim/core/polygon.h>
#include <seraphim/object/dnn_classifier.h>

static bool main_loop = true;

static struct option long_opts[] = {
    { "camera", required_argument, 0, 'i' }, { "confidence_threshold", required_argument, 0, 't' },
    { "config", required_argument, 0, 'c' }, { "model", required_argument, 0, 'm' },
    { "help", no_argument, 0, 'h' },         { 0, 0, 0, 0 }
};

static char const *long_opts_desc[] = { "Camera index", "Detection threshold", "Model config path",
                                        "Model path", "Show help" };

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

    printf("%s\n\n", "dnn_classifier [flags]");
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
    float confidence_threshold = 0;
    std::string model_path;
    std::string model_config_path;
    sph::object::DNNClassifier classifier;
    std::vector<sph::object::Classifier::Prediction> predictions;
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
    while ((opt = getopt_long(argc, argv, "i:t:c:m:h", long_opts, &long_index)) != -1) {
        switch (opt) {
        case 'i':
            camera_index = std::stoi(optarg);
            break;
        case 't':
            confidence_threshold = std::stof(optarg);
            break;
        case 'c':
            model_config_path = std::string(optarg);
            break;
        case 'm':
            model_path = std::string(optarg);
            break;
        case 'h':
            print_usage(1);
            return 0;
        default:
            print_usage(0);
            return 1;
        }
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

    if (!classifier.read_net(model_path, model_config_path)) {
        std::cout << "[ERROR Failed to read net" << std::endl;
        return 1;
    }
    classifier.set_target(sph::core::IComputable::TARGET_CPU);

    // set parameters for MobileNet V2 (2018_03_29)
    // see https://github.com/opencv/opencv/wiki/TensorFlow-Object-Detection-API
    // see https://github.com/opencv/opencv/blob/master/samples/dnn/models.yml
    sph::object::DNNClassifier::BlobParameters params = {};
    params.scalefactor = 1.0;
    params.size = cv::Size(300, 300);
    params.mean = cv::Scalar();
    params.swap_rb = true;
    params.crop = false;
    classifier.set_blob_parameters(params);

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
        if (!sph::core::Mat2Image(frame, image)) {
            std::cout << "[ERROR] Failed to convert Mat to Image" << std::endl;
            continue;
        }

        classifier.predict(image, predictions);
        process_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::high_resolution_clock::now() - t_frame_captured)
                           .count();

        if (predictions.size() > 0) {
            for (auto &pred : predictions) {
                if (pred.confidence < confidence_threshold) {
                    continue;
                }

                // skip id 0 (background)
                if (pred.class_id == 0) {
                    continue;
                }

                cv::Scalar color(0, 0, 0);
                if (pred.class_id <= 30) {
                    color = cv::Scalar(255, 0, 0);
                } else if (pred.class_id <= 60) {
                    color = cv::Scalar(0, 255, 0);
                } else if (pred.class_id <= 90) {
                    color = cv::Scalar(0, 0, 255);
                }

                cv::rectangle(frame,
                              cv::Rect(pred.poly.bl().x, pred.poly.bl().y, pred.poly.width(),
                                       pred.poly.height()),
                              cv::Scalar(0, 255, 0), 2);
                std::string label = cv::format("%.2f", pred.confidence);
                // if (MOBILENET_V2_COCO_2018_03_29.find(classId) !=
                // MOBILENET_V2_COCO_2018_03_29.end()) {
                //    label = MOBILENET_V2_COCO_2018_03_29.at(classId) + ": " + label;
                //}
                int baseLine;
                int top = pred.poly.tl().y;
                int left = pred.poly.tl().x;
                cv::Size labelSize =
                    cv::getTextSize(label, cv::FONT_HERSHEY_DUPLEX, 1.0, 1, &baseLine);
                top = cv::max(top, labelSize.height);
                rectangle(frame, cv::Point(left, top),
                          cv::Point(left + labelSize.width, top + labelSize.height + baseLine),
                          color, cv::FILLED);
                putText(frame, label, cv::Point(left, top + labelSize.height),
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

        cv::imshow("DNN Classifier", frame);
        cv::waitKey(1);
    }
}
