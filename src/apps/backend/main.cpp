/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <chrono>
#include <csignal>
#include <getopt.h>
#include <iostream>
#include <poll.h>
#include <thread>
#include <vector>

#include <Seraphim.pb.h>
#include <seraphim/car/linear_lane_detector.hpp>
#include <seraphim/face/lbf_facemark_detector.hpp>
#include <seraphim/face/lbp_face_detector.hpp>
#include <seraphim/face/lbp_face_recognizer.hpp>
#include <seraphim/face/utils.hpp>
#include <seraphim/ipc.hpp>
#include <seraphim/memory.hpp>
#include <seraphim/object/dnn_detector.hpp>

#include "car/lane_detector_service.hpp"
#include "config_store.hpp"
#include "face/face_detector_service.hpp"
#include "face/face_recognizer_service.hpp"
#include "face/facemark_detector_service.hpp"
#include "object/detector_service.hpp"
#include "shm_server.hpp"
#include "tcp_server.hpp"

using namespace sph::backend;
using namespace sph::ipc;

static bool server_running = false;

static auto lane_detector = std::make_shared<sph::car::LinearLaneDetector>();
static auto face_detector = std::make_shared<sph::face::LBPFaceDetector>();
static auto face_recognizer = std::make_shared<sph::face::LBPFaceRecognizer>();
static auto facemark_detector = std::make_shared<sph::face::LBFFacemarkDetector>();
static auto object_detector = std::make_shared<sph::object::DNNDetector>();

static auto lane_detector_service = std::make_shared<sph::car::LaneDetectorService>(lane_detector);
static auto face_detector_service = std::make_shared<sph::face::FaceDetectorService>(face_detector);
static auto face_recognizer_service = std::make_shared<sph::face::FaceRecognizerService>(
    face_detector, facemark_detector, face_recognizer);
static auto facemark_detector_service =
    std::make_shared<sph::face::FacemarkDetectorService>(face_detector, facemark_detector);
static auto object_detector_service =
    std::make_shared<sph::object::DetectorService>(object_detector);

void signal_handler(int signal) {
    switch (signal) {
    case SIGINT:
        std::cout << "caught SIGINT" << std::endl;
        server_running = false;
        break;
    default:
        std::cout << "unknown signal: " << signal << std::endl;
        break;
    }
}

static struct option long_opts[] = { { "config", required_argument, 0, 'c' },
                                     { "help", no_argument, 0, 'h' },
                                     { 0, 0, 0, 0 } };

static char const *long_opts_desc[] = { "Path to the configuration file (default: ./seraphim.conf)",
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

    printf("%s\n\n", "seraphim-server [flags]");
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

int main(int argc, char **argv) {
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    std::string config_path = "seraphim.conf";
    std::list<std::unique_ptr<Server>> servers;
    std::string val;
    std::string val2;

    // register signal handler
    signal(SIGINT, signal_handler);

    int opt = 0;
    int long_index = 0;
    while ((opt = getopt_long(argc, argv, "c:h", long_opts, &long_index)) != -1) {
        switch (opt) {
        case 'c':
            config_path = std::string(optarg);
            break;
        case 'h':
            print_usage(1);
            return 0;
        default:
            print_usage(0);
            return 1;
        }
    }

    // load config
    if (!ConfigStore::Instance().open(config_path)) {
        std::cout << "[ERROR] Failed to load config from: " << config_path << std::endl;
        return 1;
    }

    // load pretrained models
    val = ConfigStore::Instance().get_value("face_cascade");
    if (val.empty()) {
        std::cout << "[ERROR] Missing conf key: face_cascade" << std::endl;
        return 1;
    } else {
        if (!face_detector->load_face_cascade(val)) {
            std::cout << "[ERROR] Failed to load face cascade from: " << val << std::endl;
            return 1;
        }
    }

    val = ConfigStore::Instance().get_value("face_facemark_model");
    if (val.empty()) {
        std::cout << "[ERROR] Missing conf key: face_facemark_model" << std::endl;
        return 1;
    } else {
        if (!facemark_detector->load_facemark_model(val)) {
            std::cout << "[ERROR] Failed to load facemark model from: " << val << std::endl;
            return 1;
        }
    }

    val = ConfigStore::Instance().get_value("object_net_model");
    val2 = ConfigStore::Instance().get_value("object_net_config");
    if (val.empty() || val2.empty()) {
        std::cout << "[ERROR] Missing conf key: object_net_model|object_net_config" << std::endl;
        return 1;
    } else {
        if (!object_detector->read_net(val, val2)) {
            std::cout << "[ERROR] Failed to load object net from: model: " << val
                      << ", config: " << val2 << std::endl;
            return 1;
        }
    }

    val = ConfigStore::Instance().get_value("compute_target");
    if (!val.empty()) {
        sph::Computable::Target target = sph::Computable::Target::CPU;
        if (val == "CPU") {
            target = sph::Computable::Target::CPU;
        } else if (val == "OPENCL") {
            target = sph::Computable::Target::OPENCL;
        } else if (val == "OPENCLFP16") {
            target = sph::Computable::Target::OPENCL_FP16;
        } else {
            std::cout << "[WARN] Invalid compute target, fallback to CPU" << std::endl;
        }

        if (!lane_detector->set_target(target)) {
            std::cout << "[WARN] Failed to set Lane Detector target to: " << val << std::endl;
        }
        if (!face_detector->set_target(target)) {
            std::cout << "[WARN] Failed to set Face Detector target to: " << val << std::endl;
        }
        if (!face_recognizer->set_target(target)) {
            std::cout << "[WARN] Failed to set Face Recognizer target to: " << val << std::endl;
        }
        if (!object_detector->set_target(target)) {
            std::cout << "[WARN] Failed to set Object Detector target to: " << val << std::endl;
        }
    }

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
    lane_detector->set_parameters(params);

    // start servers
    server_running = true;

    val = ConfigStore::Instance().get_value("shm_server_uri");
    if (!val.empty()) {
        std::cout << "Creating SHM server (uri: " << val << ")" << std::endl;
        try {
            auto transport = TransportFactory::Instance().create(val);
            auto shared = sph::convert_shared<SharedMemoryTransport>(transport);
            auto server = std::unique_ptr<SharedMemoryServer>(new SharedMemoryServer(shared));
            servers.emplace_back(std::move(server));
        } catch (const std::exception &e) {
            std::cout << "Failed to create SHM segment: " << e.what() << std::endl;
        }
    }

    val = ConfigStore::Instance().get_value("tcp_server_uri");
    if (!val.empty()) {
        std::cout << "Creating TCP server (uri: " << val << ")" << std::endl;
        try {
            auto transport = TransportFactory::Instance().create(val);
            auto shared = sph::convert_shared<TCPTransport>(transport);
            auto server = std::unique_ptr<TCPServer>(new TCPServer(shared));
            servers.emplace_back(std::move(server));
        } catch (const std::exception &e) {
            std::cout << "Failed to create TCP server: " << e.what() << std::endl;
        }
    }

    // register the event handlers on all servers
    for (const auto &server : servers) {
        server->register_event_handler(Server::EVENT_CLIENT_CONNECTED, [](void *) {
            std::cout << "** Client connected" << std::endl;
        });
        server->register_event_handler(Server::EVENT_CLIENT_DISCONNECTED, [](void *) {
            std::cout << "** Client disconnected" << std::endl;
        });
        server->register_event_handler(Server::EVENT_MESSAGE_INBOUND, [](void *data) {
            Seraphim::Message *msg = reinterpret_cast<Seraphim::Message *>(data);
            std::cout << ">> Message inbound" << std::endl
                      << "   id=" << msg->id() << std::endl
                      << "   size=" << msg->ByteSizeLong() << std::endl;
        });
        server->register_event_handler(Server::EVENT_MESSAGE_OUTBOUND, [](void *data) {
            Seraphim::Message *msg = reinterpret_cast<Seraphim::Message *>(data);
            std::cout << "<< Message outbound" << std::endl
                      << "   id=" << msg->id() << std::endl
                      << "   size=" << msg->ByteSizeLong() << std::endl;
        });
        server->register_event_handler(Server::EVENT_MESSAGE_HANDLED, [](void *data) {
            Seraphim::Message *msg = reinterpret_cast<Seraphim::Message *>(data);
            std::cout << "== Message handled" << std::endl
                      << "   id=" << msg->id() << std::endl
                      << "   response status=" << msg->res().status() << std::endl;
        });
    }

    // register the services on all servers
    for (const auto &server : servers) {
        server->register_service(lane_detector_service);
        server->register_service(face_detector_service);
        server->register_service(face_recognizer_service);
        server->register_service(facemark_detector_service);
        server->register_service(object_detector_service);
    }

    // start the servers
    auto iter = servers.begin();
    auto end = servers.end();
    while (iter != end) {
        if (!iter->get()->run()) {
            std::cout << "Failed to run server: " << strerror(errno) << std::endl;
            iter = servers.erase(iter);
            continue;
        }

        iter++;
    }

    while (server_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Stopping servers" << std::endl;
    for (const auto &serv : servers) {
        serv->terminate();
    }

    return 0;
}
