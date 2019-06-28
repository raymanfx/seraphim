#include <QPixmap>

#include "QImageProvider.h"

QImageProvider::QImageProvider(QObject *parent)
    : QObject(parent), QQuickImageProvider(QQuickImageProvider::Pixmap) {
    // dummy
    mImage = QImage();
}

QPixmap QImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) {
    (void)id;
    (void)requestedSize;
    *size = mImage.size();
    return QPixmap::fromImage(mImage);
}

void QImageProvider::framePainted() {
    // dummy
}

bool QImageProvider::show(const QImage &image) {
    // shallow copy, must be repainted immediately
    mImage = image;

    emit repaint();
    return true;
}

QImage QImageProvider::QImageFromBuffer(void *buffer, const size_t &size, const uint32_t &width,
                                        const uint32_t &height, const uint32_t &fourcc) {
    const uchar *bytes = reinterpret_cast<uchar *>(buffer);

    if (width == 0 || height == 0 || fourcc == 0) {
        return QImage();
    }

    // https://doc.qt.io/qt-5/qvideoframe.html#PixelFormat-enum
    switch (fourcc) {
    case QImageProvider::fourcc('r', 'a', 'w', ' '):
        return QImage(bytes, static_cast<int>(width), static_cast<int>(height),
                      QImage::Format_RGB888);
    case QImageProvider::fourcc('M', 'J', 'P', 'G'):
        return QImage::fromData(bytes, static_cast<int>(size), "mjpeg");
    default:
        return QImage();
    }
}
