#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include <QTextStream>
#include <QDir>

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

    loadStylesheet(app);

    omnium::MainWindow w;
    w.resize(1400, 900);
    w.show();

    return app.exec();
}
