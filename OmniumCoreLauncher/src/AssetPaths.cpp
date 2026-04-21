#include "AssetPaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

namespace omnium::AssetPaths {

QString resolve(const QString& relative) {
    const QString appDir = QCoreApplication::applicationDirPath();

    const QStringList candidates = {
        appDir + QStringLiteral("/assets/")    + relative,
        appDir + QStringLiteral("/../assets/") + relative,
    };
    for (const QString& c : candidates) {
        if (QFileInfo::exists(c))
            return QDir::cleanPath(c);
    }

    const QString embedded = QStringLiteral(":/assets/") + relative;
    if (QFileInfo::exists(embedded))
        return embedded;

    return QString();
}

QUrl resolveUrl(const QString& relative) {
    const QString p = resolve(relative);
    if (p.isEmpty())
        return QUrl(QStringLiteral("qrc:/assets/") + relative);
    if (p.startsWith(QLatin1Char(':')))
        return QUrl(QStringLiteral("qrc") + p);
    return QUrl::fromLocalFile(p);
}

} // namespace omnium::AssetPaths
