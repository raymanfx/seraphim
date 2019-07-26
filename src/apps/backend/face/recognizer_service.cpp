/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <seraphim/face/utils.h>
#include <utils.h>

#include "recognizer_service.h"

using namespace sph::face;

RecognizerService::RecognizerService(std::shared_ptr<sph::face::LBPDetector> detector,
                                     std::shared_ptr<sph::face::IRecognizer> recognizer) {
    m_detector = detector;
    m_recognizer = recognizer;
}

RecognizerService::~RecognizerService() {
    // dummy
}

bool RecognizerService::handle_request(const Seraphim::Request &req, Seraphim::Response &res) {
    if (!req.has_face() || !req.face().has_recognizer()) {
        return false;
    }

    if (req.face().recognizer().has_training()) {
        return handle_training_request(
            req.face().recognizer().training(),
            *res.mutable_face()->mutable_recognizer()->mutable_training());
    } else if (req.face().recognizer().has_recognition()) {
        return handle_recognition_request(
            req.face().recognizer().recognition(),
            *res.mutable_face()->mutable_recognizer()->mutable_recognition());
    }

    return false;
}

bool RecognizerService::handle_training_request(
    const Seraphim::Face::Recognizer::TrainingRequest &req,
    Seraphim::Face::Recognizer::TrainingResponse &res) {
    cv::Mat image;
    std::vector<cv::Mat> images;
    std::vector<int> labels;

    if (!sph::backend::Image2DtoMat(req.image(), image)) {
        return false;
    }

    labels.push_back(req.label());

    // align the image at the eyes before training
    cv::Mat alignedFace;
    cv::RotatedRect alignedROI;

    image.copyTo(alignedFace);

    // compute centers of the eyes
    std::vector<cv::Rect> faces;
    std::vector<IDetector::Facemarks> facemarks;
    std::vector<cv::Point2f> eyes;
    m_detector->detect_faces(image, faces);

    if (faces.size() != 1) {
        res.set_label(-1);
    } else {
        Seraphim::Types::Region2D *face = res.mutable_face();

        face->set_x(faces.at(0).x);
        face->set_y(faces.at(0).y);
        face->set_w(faces.at(0).width);
        face->set_h(faces.at(0).height);

        m_detector->detect_facemarks(image, faces, facemarks);

        // collect the eye point positions from the face
        for (const auto &landmark_set : facemarks[0].landmarks) {
            if (landmark_set.first == IDetector::FacemarkType::LEFT_EYE || landmark_set.first == IDetector::FacemarkType::RIGHT_EYE) {
                cv::Point2f p;
                for (const auto &point : landmark_set.second) {
                    p.x += point.x;
                    p.y += point.y;
                }
                p.x /= landmark_set.second.size();
                p.y /= landmark_set.second.size();
                eyes.push_back(p);
                break;
            }
        }

        if (eyes.size() == 0) {
            std::cout << "Could not detect eyes, aborting training" << std::endl;
            return false;
        }

        double angle = sph::face::align_face(alignedFace, eyes);
        std::cout << "rot angle: " << angle << std::endl;

        // rotate the face ROI by the same angle
        alignedROI = cv::RotatedRect((faces[0].br() + faces[0].tl()) * 0.5, faces[0].size(),
                                     static_cast<float>(angle));

        // perform final cropping of the image using the minimal upright
        // bounding rect
        alignedFace = alignedFace(alignedROI.boundingRect());

        images.push_back(alignedFace);
        m_recognizer->update(images, labels, req.invalidate());
        res.set_label(req.label());
    }

    return true;
}

bool RecognizerService::handle_recognition_request(
    const Seraphim::Face::Recognizer::RecognitionRequest &req,
    Seraphim::Face::Recognizer::RecognitionResponse &res) {
    cv::Mat image;
    std::vector<cv::Rect> faces;
    cv::Rect2i roi;
    std::vector<sph::face::IRecognizer::Prediction> preds;

    if (!sph::backend::Image2DtoMat(req.image(), image)) {
        return false;
    }

    roi = cv::Rect2i(0, 0, image.cols, image.rows);

    if (req.has_roi()) {
        roi.x = req.roi().x();
        roi.y = req.roi().y();
        roi.width = req.roi().w();
        roi.height = req.roi().h();
    }

    m_detector->detect_faces(image(roi), faces);
    for (size_t i = 0; i < faces.size(); i++) {
        m_recognizer->predict(image(faces[i]), preds);
        Seraphim::Types::Region2D *roi = res.add_rois();
        for (size_t j = 0; j < preds.size(); j++) {
            // filter results if a global threshold is set
            if (req.confidence() > 0.0 && preds.at(j).confidence < req.confidence()) {
                res.mutable_rois()->RemoveLast();
                continue;
            }

            res.add_labels(preds.at(j).label);
            res.add_distances(preds.at(j).confidence);
            roi->set_x(faces.at(i).x);
            roi->set_y(faces.at(i).y);
            roi->set_w(faces.at(i).width);
            roi->set_h(faces.at(i).height);
        }
    }

    return true;
}
