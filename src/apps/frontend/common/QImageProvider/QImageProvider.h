#ifndef QIMAGEPROVIDER_H
#define QIMAGEPROVIDER_H

#include <QImage>
#include <QObject>
#include <QQuickImageProvider>
#include <cstring>

class QImageProvider : public QObject, public QQuickImageProvider {
    Q_OBJECT
public:
    explicit QImageProvider(QObject *parent = nullptr);

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

signals:
    void repaint();

public slots:
    void framePainted();

public:
    bool show(const QImage &image);

private:
    QImage mImage;
};

#endif // QIMAGEPROVIDER_H
