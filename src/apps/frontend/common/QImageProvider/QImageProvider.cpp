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
    // since this is in fact just a shallow copy, we must not try to free the buffer in the
    // destructor of the mImage instance in this class!
    mImage =
        QImage(image.bits(), image.width(), image.height(), image.bytesPerLine(), image.format());

    emit repaint();
    return true;
}
