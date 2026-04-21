#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QTextStream>

#include "AssetPaths.h"
#include "MainWindow.h"

static void loadStylesheet(QApplication& app) {
    QFile f(QStringLiteral(":/styles/midnight.qss"));
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&f);
        app.setStyleSheet(in.readAll());
    }
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QApplication::setApplicationName(QStringLiteral("OmniumCore Launcher"));
    QApplication::setApplicationVersion(QStringLiteral(OMNIUM_VERSION));
    QApplication::setOrganizationName(QStringLiteral("OmniumCore"));
    QApplication::setOrganizationDomain(QStringLiteral("omniumcore.app"));

    // App icon — falls back to embedded resource if user hasn't replaced the file.
    const QString iconPath = omnium::AssetPaths::resolve(
        QStringLiteral("icons/app_icon.svg"));
    if (!iconPath.isEmpty())
        QApplication::setWindowIcon(QIcon(iconPath));

    loadStylesheet(app);

    omnium::MainWindow w;
    w.resize(1400, 900);
    w.show();

    return app.exec();
}
