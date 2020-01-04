/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_RECOGNIZER_SERVICE_HPP
#define SPH_FACE_RECOGNIZER_SERVICE_HPP

#include <FaceRecognizer.pb.h>
#include <seraphim/face/face_detector.hpp>
#include <seraphim/face/face_recognizer.hpp>
#include <seraphim/face/facemark_detector.hpp>

#include "../service.hpp"

namespace sph {
namespace face {

class FaceRecognizerService : public sph::backend::Service {
public:
    explicit FaceRecognizerService(std::shared_ptr<sph::face::FaceDetector> face_detector,
                                   std::shared_ptr<sph::face::FacemarkDetector> facemark_detector,
                                   std::shared_ptr<sph::face::FaceRecognizer> face_recognizer);
    ~FaceRecognizerService() override;

    bool handle_request(const Seraphim::Request &req, Seraphim::Response &res) override;
    bool handle_training_request(const Seraphim::Face::FaceRecognizer::TrainingRequest &req,
                                 Seraphim::Face::FaceRecognizer::TrainingResponse &res);
    bool handle_recognition_request(const Seraphim::Face::FaceRecognizer::PredictionRequest &req,
                                    Seraphim::Face::FaceRecognizer::PredictionResponse &res);

private:
    std::shared_ptr<sph::face::FaceDetector> m_face_detector;
    std::shared_ptr<sph::face::FaceRecognizer> m_face_recognizer;
    std::shared_ptr<sph::face::FacemarkDetector> m_facemark_detector;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_RECOGNIZER_SERVICE_HPP
