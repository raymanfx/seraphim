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

    /**
     * @brief QImageFromBuffer Create a QImage from a raw buffer.
     * @param buffer The start of the continous buffer.
     * @param size Size of the buffer.
     * @param width Width of the image in pixels.
     * @param height Height of the image in pixels.
     * @param fourcc Four character code format.
     * @return The QImage wrapper (owns shallow clone of the raw buffer).
     */
    static QImage QImageFromBuffer(void *buffer, const size_t &size, const uint32_t &width,
                                   const uint32_t &height, const uint32_t &fourcc);

    /**
     * @brief fourcc Compute the four character code for a string.
     * @param c1 e.g. 'M'
     * @param c2 e.g. 'J'
     * @param c3 e.g. 'P'
     * @param c4 e.g. 'G'
     * @return The four character code.
     */
    static inline constexpr uint32_t fourcc(const char &c1, const char &c2, const char &c3,
                                            const char &c4) {
        return ((static_cast<uint32_t>(c1) << 0) | (static_cast<uint32_t>(c2) << 8) |
                (static_cast<uint32_t>(c3) << 16) | (static_cast<uint32_t>(c4) << 24));
    }

    /**
     * @brief fourcc_to_string Convert a four character code to its string representation.
     * @param fourcc The four character code.
     * @return The string representation.
     */
    static inline std::string fourcc_to_string(uint32_t fourcc) {
        char str[4];
        std::strncpy(str, reinterpret_cast<char *>(&fourcc), 4);
        return std::string(str);
    }

private:
    QImage mImage;
};

#endif // QIMAGEPROVIDER_H
