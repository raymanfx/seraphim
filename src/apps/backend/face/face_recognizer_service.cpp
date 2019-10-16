/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <seraphim/face/utils.h>
#include <seraphim/iop/opencv/mat.h>
#include <utils.h>

#include "face_recognizer_service.h"

using namespace sph;
using namespace sph::face;

FaceRecognizerService::FaceRecognizerService(
    std::shared_ptr<sph::face::FaceDetector> face_detector,
    std::shared_ptr<sph::face::FacemarkDetector> facemark_detector,
    std::shared_ptr<sph::face::FaceRecognizer> face_recognizer) {
    m_face_detector = face_detector;
    m_face_recognizer = face_recognizer;
    m_facemark_detector = facemark_detector;
}

FaceRecognizerService::~FaceRecognizerService() {
    // dummy
}

bool FaceRecognizerService::handle_request(const Seraphim::Request &req, Seraphim::Response &res) {
    if (req.inner().Is<Seraphim::Face::FaceRecognizer::TrainingRequest>()) {
        Seraphim::Face::FaceRecognizer::TrainingRequest inner_req;
        Seraphim::Face::FaceRecognizer::TrainingResponse inner_res;

        req.inner().UnpackTo(&inner_req);
        if (handle_training_request(inner_req, inner_res)) {
            res.mutable_inner()->PackFrom(inner_res);
            return true;
        }
    } else if (req.inner().Is<Seraphim::Face::FaceRecognizer::RecognitionRequest>()) {
        Seraphim::Face::FaceRecognizer::RecognitionRequest inner_req;
        Seraphim::Face::FaceRecognizer::RecognitionResponse inner_res;

        req.inner().UnpackTo(&inner_req);
        if (handle_recognition_request(inner_req, inner_res)) {
            res.mutable_inner()->PackFrom(inner_res);
            return true;
        }
    }

    return false;
}

bool FaceRecognizerService::handle_training_request(
    const Seraphim::Face::FaceRecognizer::TrainingRequest &req,
    Seraphim::Face::FaceRecognizer::TrainingResponse &res) {
    Image image;
    std::vector<Image> images;
    std::vector<Polygon<int>> faces;
    cv::Mat mat;
    std::vector<int> labels;

    if (!sph::backend::Image2DtoMat(req.image(), mat)) {
        return false;
    }

    image = sph::iop::cv::MatFacility::to_image(mat);
    if (image.empty()) {
        return false;
    }

    labels.push_back(req.label());

    // align the image at the eyes before training
    cv::Mat alignedFace;
    cv::RotatedRect alignedROI;

    mat.copyTo(alignedFace);

    // compute centers of the eyes
    std::vector<FacemarkDetector::Facemarks> facemarks;
    std::vector<cv::Point2f> eyes;
    m_face_detector->detect_faces(image, faces);

    if (faces.size() != 1) {
        res.set_label(-1);
    } else {
        Seraphim::Types::Region2D *face = res.mutable_face();

        face->set_x(faces.at(0).bl().x);
        face->set_y(faces.at(0).bl().y);
        face->set_w(faces.at(0).width());
        face->set_h(faces.at(0).height());

        // search in the region of the previously detected face
        m_facemark_detector->detect_facemarks(image, faces, facemarks);
        if (faces.size() == 0 || facemarks.size() == 0) {
            return false;
        }

        // collect the eye point positions from the face
        for (const auto &landmark_set : facemarks[0].landmarks) {
            if (landmark_set.first == FacemarkDetector::FacemarkType::LEFT_EYE ||
                landmark_set.first == FacemarkDetector::FacemarkType::RIGHT_EYE) {
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
        alignedROI = cv::RotatedRect((cv::Point(faces[0].br().x, faces[0].br().y) +
                                      cv::Point(faces[0].tl().x, faces[0].tl().y)) *
                                         0.5,
                                     cv::Size(faces[0].width(), faces[0].height()),
                                     static_cast<float>(angle));

        if (alignedROI.boundingRect().width > alignedFace.size().width ||
            alignedROI.boundingRect().height > alignedFace.size().height) {
            // the input image is too small to do proper cropping
            return false;
        }

        // perform final cropping of the image using the minimal upright
        // bounding rect
        alignedFace = alignedFace(alignedROI.boundingRect());

        image = sph::iop::cv::MatFacility::to_image(alignedFace);
        if (image.empty()) {
            return false;
        }
        images.push_back(image);
        m_face_recognizer->update(images, labels, req.invalidate());
        res.set_label(req.label());
    }

    return true;
}

bool FaceRecognizerService::handle_recognition_request(
    const Seraphim::Face::FaceRecognizer::RecognitionRequest &req,
    Seraphim::Face::FaceRecognizer::RecognitionResponse &res) {
    Image image;
    std::vector<Polygon<int>> faces;
    cv::Mat mat;
    cv::Rect2i roi;
    std::vector<sph::face::FaceRecognizer::Prediction> preds;

    if (!sph::backend::Image2DtoMat(req.image(), mat)) {
        return false;
    }

    roi = cv::Rect2i(0, 0, mat.cols, mat.rows);

    if (req.has_roi()) {
        roi.x = req.roi().x();
        roi.y = req.roi().y();
        roi.width = req.roi().w();
        roi.height = req.roi().h();
    }

    mat = mat(roi);
    image = sph::iop::cv::MatFacility::to_image(mat);
    if (image.empty()) {
        return false;
    }

    m_face_detector->detect_faces(image, faces);
    for (size_t i = 0; i < faces.size(); i++) {
        cv::Rect face_region(faces[i].bl().x, faces[i].bl().y, faces[i].width(), faces[i].height());
        image = sph::iop::cv::MatFacility::to_image(mat(face_region));
        if (image.empty()) {
            return false;
        }

        m_face_recognizer->predict(image, preds);
        Seraphim::Types::Region2D *roi = res.add_rois();
        for (size_t j = 0; j < preds.size(); j++) {
            // filter results if a global threshold is set
            if (req.confidence() > 0.0 && preds.at(j).confidence < req.confidence()) {
                res.mutable_rois()->RemoveLast();
                continue;
            }

            res.add_labels(preds.at(j).label);
            res.add_distances(preds.at(j).confidence);
            roi->set_x(faces.at(0).bl().x);
            roi->set_y(faces.at(0).bl().y);
            roi->set_w(faces.at(0).width());
            roi->set_h(faces.at(0).height());
        }
    }

    return true;
}
