/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_OBJECT_CLASSIFIER_SERVICE_H
#define SPH_OBJECT_CLASSIFIER_SERVICE_H

#include <ObjectClassifier.pb.h>
#include <seraphim/object/classifier.h>

#include "../service.h"

namespace sph {
namespace object {

class ClassifierService : public sph::backend::IService {
public:
    explicit ClassifierService(std::shared_ptr<sph::object::Classifier> recognizer);

    bool handle_request(const Seraphim::Request &req, Seraphim::Response &res) override;
    bool
    handle_classification_request(const Seraphim::Object::Classifier::ClassificationRequest &req,
                                  Seraphim::Object::Classifier::ClassificationResponse &res);

private:
    std::shared_ptr<sph::object::Classifier> m_recognizer;
};

} // namespace object
} // namespace sph

#endif // SPH_OBJECT_CLASSIFIER_SERVICE_H
