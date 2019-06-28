#include <QGuiApplication>

#include <ui/MainWindow.h>

int main(int argc, char *argv[]) {
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    MainWindow window;
    if (!window.load())
        return -1;

    return app.exec();
}
