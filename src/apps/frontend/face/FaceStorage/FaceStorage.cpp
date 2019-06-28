#include "FaceStorage.h"
#include <iostream>

FaceStorage::FaceStorage(QObject *parent) : QQuickImageProvider(QQuickImageProvider::Pixmap) {
    // dummy
    (void)parent;
}

QPixmap FaceStorage::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) {
    int label = id.toInt();

    auto it = mFaces.find(label);
    if (it == mFaces.end()) {
        *size = requestedSize;
        return QPixmap::fromImage(QImage(requestedSize, QImage::Format_RGB32));
    }

    *size = mFaces[label].size();
    return QPixmap::fromImage(mFaces[label]);
}

bool FaceStorage::getFace(const int &label, QImage &face) {
    auto it = mFaces.find(label);
    if (it == mFaces.end())
        return false;

    face = mFaces[label];
    return true;
}

void FaceStorage::setFace(const int &label, QImage &face) {
    mFaces[label] = face.copy();
}
