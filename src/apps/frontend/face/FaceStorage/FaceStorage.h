#ifndef FACESTORAGE_H
#define FACESTORAGE_H

#include <map>

#include <QImage>
#include <QObject>
#include <QQuickImageProvider>

class FaceStorage : public QObject, public QQuickImageProvider {
    Q_OBJECT
public:
    explicit FaceStorage(QObject *parent = nullptr);

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

    bool getFace(const int &label, QImage &face);
    void setFace(const int &label, QImage &face);

signals:

public slots:

private:
    std::map<int, QImage> mFaces;
};

#endif // FACESTORAGE_H
