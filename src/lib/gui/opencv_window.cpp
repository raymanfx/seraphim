/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <thread>

#include <opencv2/highgui.hpp>

#include "seraphim/except.h"
#include "seraphim/gui/opencv_window.h"
#include "seraphim/iop/opencv/mat.h"

using namespace sph;
using namespace sph::gui;

OpenCVWindow::OpenCVWindow(const std::string &title) : m_title(title) {
    cv::namedWindow(title);
}

OpenCVWindow::~OpenCVWindow() {
    cv::destroyWindow(m_title);
}

void OpenCVWindow::show(const sph::Image &img) {
    cv::Mat mat;
    int key;

    mat = sph::iop::cv::from_image(img);
    if (mat.empty()) {
        SPH_THROW(InvalidArgumentException, "Unsupported image format");
    }

    cv::imshow(m_title, mat);
    key = cv::waitKey(1);
}
