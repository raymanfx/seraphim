/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_RECOGNIZER_SERVICE_H
#define SPH_FACE_RECOGNIZER_SERVICE_H

#include <FaceRecognizer.pb.h>
#include <seraphim/face/detector.h>
#include <seraphim/face/recognizer.h>

#include "../service.h"

namespace sph {
namespace face {

class RecognizerService : public sph::backend::IService {
public:
    explicit RecognizerService(std::shared_ptr<sph::face::IDetector> detector,
                               std::shared_ptr<sph::face::IRecognizer> recognizer);
    ~RecognizerService() override;

    bool handle_request(const Seraphim::Request &req, Seraphim::Response &res) override;
    bool handle_training_request(const Seraphim::Face::Recognizer::TrainingRequest &req,
                                 Seraphim::Face::Recognizer::TrainingResponse &res);
    bool handle_recognition_request(const Seraphim::Face::Recognizer::RecognitionRequest &req,
                                    Seraphim::Face::Recognizer::RecognitionResponse &res);

private:
    std::shared_ptr<sph::face::IDetector> m_detector;
    std::shared_ptr<sph::face::IRecognizer> m_recognizer;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_RECOGNIZER_SERVICE_H
