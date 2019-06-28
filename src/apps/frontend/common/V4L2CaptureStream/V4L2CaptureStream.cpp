#include <cstring>

#include "V4L2CaptureStream.h"

V4L2CaptureStream::V4L2CaptureStream() {
    mDevicePath = "/dev/video0";

    mFourcc = 0;
    mWidth = 0;
    mHeight = 0;
}

bool V4L2CaptureStream::open() {
    return mDevice.open(mDevicePath);
}

bool V4L2CaptureStream::retrieve(struct Buffer &buf) {
    V4L2Device::Buffer v4l2buf;

    if (!mDevice.retrieve(v4l2buf)) {
        return false;
    }

    buf.start = v4l2buf.start;
    buf.size = v4l2buf.length;
    buf.bytesused = v4l2buf.bytesused;
    buf.format.width = mWidth;
    buf.format.height = mHeight;
    buf.format.fourcc = mFourcc;
    return true;
}

bool V4L2CaptureStream::open(const std::string &path) {
    mDevicePath = path;
    return open();
}

bool V4L2CaptureStream::open(const int &index) {
    mDevicePath = "/dev/video" + std::to_string(index);
    return open();
}

bool V4L2CaptureStream::setFourcc(const uint32_t &fourcc) {
    struct v4l2_format fmt = {};

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (mDevice.ioctl(VIDIOC_G_FMT, &fmt) == -1) {
        return false;
    }

    fmt.fmt.pix.pixelformat = fourcc;
    if (mDevice.ioctl(VIDIOC_S_FMT, &fmt) == -1) {
        return false;
    }

    mFourcc = fourcc;
    return true;
}

bool V4L2CaptureStream::setWidth(const uint32_t &width) {
    struct v4l2_format fmt = {};

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (mDevice.ioctl(VIDIOC_G_FMT, &fmt) == -1) {
        return false;
    }

    fmt.fmt.pix.width = width;
    if (mDevice.ioctl(VIDIOC_S_FMT, &fmt) == -1) {
        return false;
    }

    mWidth = width;
    return true;
}

bool V4L2CaptureStream::setHeight(const uint32_t &height) {
    struct v4l2_format fmt = {};

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (mDevice.ioctl(VIDIOC_G_FMT, &fmt) == -1) {
        return false;
    }

    fmt.fmt.pix.height = height;
    if (mDevice.ioctl(VIDIOC_S_FMT, &fmt) == -1) {
        return false;
    }

    mHeight = height;
    return true;
}
