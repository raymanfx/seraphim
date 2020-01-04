/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_RECOGNIZER_LBPH_HPP
#define SPH_FACE_RECOGNIZER_LBPH_HPP

#include <mutex>
#include <opencv2/face.hpp>
#include <opencv2/opencv.hpp>
#include <seraphim/computable.hpp>
#include <vector>

#include "face_recognizer.hpp"

namespace sph {
namespace face {

class LBPFaceRecognizer : public FaceRecognizer, sph::Computable {
public:
    LBPFaceRecognizer() = default;

    bool empty() const { return m_impl.empty(); }
    double getThreshold() const { return m_impl->getThreshold(); }
    void read(const cv::String &filename) { return m_impl->read(filename); }
    void setThreshold(double val) { return m_impl->setThreshold(val); }
    void write(cv::FileStorage &fs) const { return m_impl->write(fs); }

    void train(const std::vector<sph::CoreImage> &imgs, const std::vector<int> &labels) override;
    bool predict(const sph::Image &img, std::vector<Prediction> &preds) override;
    void update(const std::vector<sph::CoreImage> &imgs, const std::vector<int> &labels,
                bool invalidate = false) override;

    bool set_target(Target target) override;

private:
    cv::Ptr<cv::face::FaceRecognizer> m_impl = cv::face::LBPHFaceRecognizer::create();

    // internal database
    std::map<int, std::vector<cv::Mat>> m_faces;

    Target m_target = Target::CPU;
    std::mutex m_target_mutex;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_RECOGNIZER_LBPH_HPP
