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
