#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

#include "models/Game.h"

namespace omnium {

// Scans known launcher install folders + user-provided directories for executables.
// On Windows it can also query the Registry (uninstall keys) to discover installed games.
class GameScanner : public QObject {
    Q_OBJECT
public:
    explicit GameScanner(QObject* parent = nullptr);

    // Returns the default folders worth scanning (Steam, EA, Epic, GOG, Ubisoft, etc.).
    static QStringList defaultScanRoots();

    // Add a custom directory to be included in the next scan.
    void addExtraRoot(const QString& path);

public slots:
    // Synchronous scan. For larger libraries, run from a worker thread (QtConcurrent).
    QVector<Game> scan();

signals:
    void scanProgress(int current, int total, const QString& currentPath);
    void scanFinished(int gamesFound);

private:
    void scanDirectory(const QString& root, Platform hint, QVector<Game>& out);
    Platform inferPlatformFromPath(const QString& path) const;
    bool     looksLikeGameExecutable(const QString& fileName) const;

#ifdef Q_OS_WIN
    // Read uninstall registry keys (HKLM/HKCU \\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall)
    // to discover installed games + their install paths.
    QVector<Game> scanRegistry();
#endif

    QStringList m_extraRoots;
};

} // namespace omnium
