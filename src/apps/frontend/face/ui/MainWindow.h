#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <atomic>
#include <condition_variable>
#include <mutex>

#include <FaceStorage/FaceStorage.h>
#include <ICaptureStream/ICaptureStream.h>
#include <QImageProvider/QImageProvider.h>

#include <seraphim/ipc/shm_transport.h>
#include <seraphim/ipc/tcp_transport.h>

class MainWindow : public QObject {
    Q_OBJECT
public:
    explicit MainWindow(QObject *parent = nullptr);
    ~MainWindow();

    bool load();

signals:
    void training(int label, float progress);
    void recognized(int label, double confidence);
    void printDiag(QString text);
    void toggleOverlay(bool visible);

public slots:
    // timer slots
    void updateTimeout();
    void diagTimeout();

    // capture slot
    void updateBuffer(const ICaptureStream::Buffer &buf);

    // qml slots for buttons
    void faceDetectionButtonClicked();
    void facemarkDetectionButtonClicked();
    void faceButtonClicked(int label);
    void faceRecognitionButtonClicked();

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

    // FaceID trainer
    int mFaceLabel;
    int mFaceTrainingSamples;
    std::atomic<bool> mFaceDetection;
    std::atomic<bool> mFacemarkDetection;
    std::atomic<int> mFaceTraining;
    std::atomic<bool> mFaceRecognition;
    bool mFaceTrained;

    // backend worker
    void backendWork();
    std::thread mBackendWorker;
    std::atomic<bool> mBackendWorkerActive;
    std::mutex mBackendLock;
    bool mBackendFrameReady;
    std::condition_variable mBackendNotifier;
    std::atomic<bool> mBackendSync;

    // Wrapper to store faces
    std::vector<int> mFaceLabels;
    FaceStorage *mFaceStorage;

    QImage mFrame;
    std::mutex mFrameLock;
    QImage mOverlay;
    std::mutex mOverlayLock;

    std::unique_ptr<sph::ipc::ITransport> mTransport;
};

#endif // MAINWINDOW_H
