#include "workspace/WorkspaceWindow.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QFontDatabase>
#include <QTimer>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("SmartFlow");
    // Optional test font for offscreen environments without system font discovery.
    const auto fontIndex = app.arguments().indexOf("--smoke-font");
    if(app.arguments().contains("--smoke-test") && fontIndex >= 0) {
        if(fontIndex + 1 >= app.arguments().size()) return 3;
        const auto id = QFontDatabase::addApplicationFont(app.arguments().at(fontIndex + 1));
        if(id < 0) return 3;
        app.setFont(QFont(QFontDatabase::applicationFontFamilies(id).first(), 10));
    }
    smartflow::WorkspaceWindow window;
    window.show();
    QTimer smokeTimer;
    QElapsedTimer elapsed;
    if (app.arguments().contains("--smoke-test")) {
        elapsed.start();
        QObject::connect(&smokeTimer, &QTimer::timeout, &app, [&] {
            if(!window.execution().result()) {
                if(elapsed.elapsed() > 5000) app.exit(4);
                return;
            }
            if(!window.execution().result()->succeeded()) { app.exit(5); return; }
            const auto index = app.arguments().indexOf("--screenshot");
            if (index >= 0 && (index + 1 >= app.arguments().size() ||
                !window.grab().save(app.arguments().at(index + 1)))) {
                app.exit(2);
                return;
            }
            app.quit();
        });
        smokeTimer.start(50);
    }
    return app.exec();
}
