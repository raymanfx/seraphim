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
#include <seraphim/car/linear_lane_detector.h>
#include <seraphim/face/lbf_facemark_detector.h>
#include <seraphim/face/lbp_face_detector.h>
#include <seraphim/face/lbp_face_recognizer.h>
#include <seraphim/face/utils.h>
#include <seraphim/ipc.h>
#include <seraphim/object/dnn_classifier.h>

#include "car/lane_detector_service.h"
#include "config_store.h"
#include "face/face_detector_service.h"
#include "face/face_recognizer_service.h"
#include "face/facemark_detector_service.h"
#include "object/classifier_service.h"
#include "shm_server.h"
#include "tcp_server.h"

using namespace sph::backend;

static bool server_running = false;
static Seraphim::Message msg;

static std::shared_ptr<sph::car::LinearLaneDetector>
    lane_detector(new sph::car::LinearLaneDetector());
static std::shared_ptr<sph::face::LBPFaceDetector> face_detector(new sph::face::LBPFaceDetector());
static std::shared_ptr<sph::face::LBPFaceRecognizer>
    face_recognizer(new sph::face::LBPFaceRecognizer());
static std::shared_ptr<sph::face::LBFFacemarkDetector>
    facemark_detector(new sph::face::LBFFacemarkDetector(face_detector));
static std::shared_ptr<sph::object::DNNClassifier>
    object_classifier(new sph::object::DNNClassifier());

static sph::car::LaneDetectorService lane_detector_service(lane_detector);
static sph::face::FaceDetectorService face_detector_service(face_detector);
static sph::face::FaceRecognizerService face_recognizer_service(face_detector, facemark_detector,
                                                                face_recognizer);
static sph::face::FacemarkDetectorService facemark_detector_service(face_detector,
                                                                    facemark_detector);
static sph::object::ClassifierService object_classifier_service(object_classifier);

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

bool message_incoming(void *data) {
    Seraphim::Message *msg = reinterpret_cast<Seraphim::Message *>(data);
    Seraphim::Response res;
    bool status = false;

    if (!msg->has_req()) {
        std::cout << "Server: no request; ignore" << std::endl;
        msg->mutable_res()->set_status(Seraphim::Response::SEINVAL);
        return false;
    }

    std::cout << ">> Request" << std::endl << "   size=" << msg->ByteSizeLong() << std::endl;

    if (msg->req().has_car()) {
        if (msg->req().car().has_detector()) {
            status = lane_detector_service.handle_request(msg->req(), res);
        }
    } else if (msg->req().has_face()) {
        if (msg->req().face().has_face_detector()) {
            status = face_detector_service.handle_request(msg->req(), res);
        } else if (msg->req().face().has_face_recognizer()) {
            status = face_recognizer_service.handle_request(msg->req(), res);
        } else if (msg->req().face().has_facemark_detector()) {
            status = facemark_detector_service.handle_request(msg->req(), res);
        }
    } else if (msg->req().has_object()) {
        if (msg->req().object().has_classifier()) {
            status = object_classifier_service.handle_request(msg->req(), res);
        }
    }

    res.set_status(status ? Seraphim::Response::OK : Seraphim::Response::ERROR);
    res.Swap(msg->mutable_res());

    std::cout << "<< Response" << std::endl
              << "   status=" << msg->res().status() << std::endl
              << "   size=" << msg->ByteSizeLong() << std::endl;
    return true;
}

int main(int argc, char **argv) {
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    std::string config_path = "seraphim.conf";
    std::vector<std::unique_ptr<IServer>> servers;
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
        if (!object_classifier->read_net(val, val2)) {
            std::cout << "[ERROR] Failed to load object net from: model: " << val
                      << ", config: " << val2 << std::endl;
            return 1;
        }
    }

    val = ConfigStore::Instance().get_value("compute_target");
    if (!val.empty()) {
        sph::core::IComputable::target_t target = sph::core::IComputable::TARGET_CPU;
        if (val == "CPU") {
            target = sph::core::IComputable::TARGET_CPU;
        } else if (val == "OPENCL") {
            target = sph::core::IComputable::TARGET_OPENCL;
        } else if (val == "OPENCLFP16") {
            target = sph::core::IComputable::TARGET_OPENCL_FP16;
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
        if (!object_classifier->set_target(target)) {
            std::cout << "[WARN] Failed to set Object Classifier target to: " << val << std::endl;
        }
    }

    // start servers
    server_running = true;

    val = ConfigStore::Instance().get_value("shm_server_uri");
    if (!val.empty()) {
        std::cout << "Creating SHM server (uri: " << val << ")" << std::endl;
        SharedMemoryServer *server = new SharedMemoryServer();
        if (!server->init(val)) {
            std::cout << "Failed to create SHM segment: " << strerror(errno) << std::endl;
            delete server;
        } else {
            server->register_event_handler(IServer::EVENT_MESSAGE_INCOMING, message_incoming);
            std::cout << "Starting SHM server" << std::endl;
            if (!server->run()) {
                std::cout << "Failed to run SHM server: " << strerror(errno) << std::endl;
                delete server;
            } else {
                servers.push_back(std::unique_ptr<IServer>(server));
            }
        }
    }

    val = ConfigStore::Instance().get_value("tcp_server_uri");
    if (!val.empty()) {
        std::cout << "Creating TCP server (uri: " << val << ")" << std::endl;
        TCPServer *server = new TCPServer();
        if (!server->init(val)) {
            std::cout << "Failed to create TCP server: " << strerror(errno) << std::endl;
            delete server;
        } else {
            server->register_event_handler(IServer::EVENT_MESSAGE_INCOMING, message_incoming);
            std::cout << "Starting TCP server" << std::endl;
            if (!server->run()) {
                std::cout << "Failed to run TCP server: " << strerror(errno) << std::endl;
                delete server;
            } else {
                servers.push_back(std::unique_ptr<IServer>(server));
            }
        }
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
