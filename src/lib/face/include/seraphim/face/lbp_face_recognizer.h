/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_RECOGNIZER_LBPH_H
#define SPH_FACE_RECOGNIZER_LBPH_H

#include <mutex>
#include <opencv2/face.hpp>
#include <opencv2/opencv.hpp>
#include <seraphim/core/computable.h>
#include <vector>

#include "face_recognizer.h"

namespace sph {
namespace face {

class LBPFaceRecognizer : public IFaceRecognizer, sph::core::IComputable {
public:
    LBPFaceRecognizer();
    ~LBPFaceRecognizer() override;

    bool empty() const { return m_impl.empty(); }
    double getThreshold() const { return m_impl->getThreshold(); }
    void read(const cv::String &filename) { return m_impl->read(filename); }
    void setThreshold(double val) { return m_impl->setThreshold(val); }
    void write(cv::FileStorage &fs) const { return m_impl->write(fs); }

    void train(const std::vector<sph::core::Image> &imgs, const std::vector<int> &labels) override;
    bool predict(const sph::core::Image &img, std::vector<Prediction> &preds) override;
    void update(const std::vector<sph::core::Image> &imgs, const std::vector<int> &labels,
                bool invalidate = false) override;

    bool set_target(const target_t &target) override;

private:
    cv::Ptr<cv::face::FaceRecognizer> m_impl;

    // internal database
    std::map<int, std::vector<cv::Mat>> m_faces;

    target_t m_target;
    std::mutex m_target_mutex;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_RECOGNIZER_LBPH_H
