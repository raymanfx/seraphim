/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_FACE_RECOGNIZER_LBPH_H
#define SPH_FACE_RECOGNIZER_LBPH_H

#include "recognizer.h"
#include <opencv2/face.hpp>
#include <opencv2/opencv.hpp>
#include <seraphim/core/computable.h>
#include <vector>

namespace sph {
namespace face {

class LBPRecognizer : public IRecognizer, sph::core::IComputable {
public:
    LBPRecognizer();
    ~LBPRecognizer() override;

    bool empty() const { return m_impl.empty(); }
    double getThreshold() const { return m_impl->getThreshold(); }
    void read(const cv::String &filename) { return m_impl->read(filename); }
    void setThreshold(double val) { return m_impl->setThreshold(val); }
    void write(cv::FileStorage &fs) const { return m_impl->write(fs); }

    void train(cv::InputArrayOfArrays imgs, const std::vector<int> &labels) override;
    bool predict(cv::InputArray img, std::vector<Prediction> &preds) override;
    void update(cv::InputArrayOfArrays imgs, const std::vector<int> &labels,
                bool invalidate = false) override;

    bool set_target(const target_t &target) override;

private:
    cv::Ptr<cv::face::FaceRecognizer> m_impl;

    // internal database
    std::map<int, std::vector<cv::Mat>> m_faces;

    target_t m_target;
};

} // namespace face
} // namespace sph

#endif // SPH_FACE_RECOGNIZER_LBPH_H
