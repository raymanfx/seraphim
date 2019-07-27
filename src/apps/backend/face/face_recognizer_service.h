/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_RECOGNIZER_SERVICE_H
#define SPH_FACE_RECOGNIZER_SERVICE_H

#include <FaceRecognizer.pb.h>
#include <seraphim/face/face_detector.h>
#include <seraphim/face/face_recognizer.h>
#include <seraphim/face/facemark_detector.h>

#include "../service.h"

namespace sph {
namespace face {

class FaceRecognizerService : public sph::backend::IService {
public:
    explicit FaceRecognizerService(std::shared_ptr<sph::face::IFaceDetector> face_detector,
                                   std::shared_ptr<sph::face::IFacemarkDetector> facemark_detector,
                                   std::shared_ptr<sph::face::IFaceRecognizer> face_recognizer);
    ~FaceRecognizerService() override;

    bool handle_request(const Seraphim::Request &req, Seraphim::Response &res) override;
    bool handle_training_request(const Seraphim::Face::FaceRecognizer::TrainingRequest &req,
                                 Seraphim::Face::FaceRecognizer::TrainingResponse &res);
    bool handle_recognition_request(const Seraphim::Face::FaceRecognizer::RecognitionRequest &req,
                                    Seraphim::Face::FaceRecognizer::RecognitionResponse &res);

private:
    std::shared_ptr<sph::face::IFaceDetector> m_face_detector;
    std::shared_ptr<sph::face::IFaceRecognizer> m_face_recognizer;
    std::shared_ptr<sph::face::IFacemarkDetector> m_facemark_detector;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_RECOGNIZER_SERVICE_H
