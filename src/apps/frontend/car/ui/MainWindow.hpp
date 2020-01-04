#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <atomic>
#include <condition_variable>
#include <mutex>

#include <ICaptureStream/ICaptureStream.hpp>
#include <QImageProvider/QImageProvider.hpp>

#include <seraphim/ipc/shm_transport.hpp>
#include <seraphim/ipc/tcp_transport.hpp>

class MainWindow : public QObject {
    Q_OBJECT
public:
    explicit MainWindow(QObject *parent = nullptr);
    ~MainWindow();

    bool load();

signals:
    void printDiag(QString text);
    void toggleOverlay(bool visible);

public slots:
    // timer slots
    void updateTimeout();
    void diagTimeout();

    // capture slot
    void updateBuffer(const ICaptureStream::Buffer &buf);

    // qml slots for buttons
    void laneDetectionButtonClicked();

    // menu bar settings
    void toggleBackendSync(bool enable);
    bool openTransportSession(QString uri);

private:
    // main event loop
    QTimer *mMainTimer;
    long mMainLoopDuration;

    // diagnostics output loop
    QTimer *mDiagPrintTimer;
    QString mDiagBuffer;

    // QML engine
    QQmlApplicationEngine *mEngine;
    QImageProvider *mMainImageProvider;
    QImageProvider *mOverlayImageProvider;

    // capture source
    std::unique_ptr<ICaptureStream> mCaptureStream;
    ICaptureStream::Buffer mCaptureBuffer;

    std::atomic<bool> mLaneDetection;

    // backend worker
    void backendWork();
    std::thread mBackendWorker;
    std::atomic<bool> mBackendWorkerActive;
    std::mutex mBackendLock;
    bool mBackendFrameReady;
    std::condition_variable mBackendNotifier;
    std::atomic<bool> mBackendSync;

    QImage mFrame;
    std::mutex mFrameLock;
    QImage mOverlay;
    std::mutex mOverlayLock;

    std::unique_ptr<sph::ipc::Transport> mTransport;
};

#endif // MAINWINDOW_HPP
