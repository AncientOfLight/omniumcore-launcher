#include "GameScanner.h"

#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSet>

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

namespace omnium {

GameScanner::GameScanner(QObject* parent) : QObject(parent) {}

QStringList GameScanner::defaultScanRoots() {
    QStringList roots;
#ifdef Q_OS_WIN
    // Common install folders for Windows launchers.
    const QStringList drives = {QStringLiteral("C:/"), QStringLiteral("D:/"),
                                QStringLiteral("E:/"), QStringLiteral("F:/")};
    const QStringList suffixes = {
        QStringLiteral("Program Files (x86)/Steam/steamapps/common"),
        QStringLiteral("Program Files/Steam/steamapps/common"),
        QStringLiteral("Program Files/EA Games"),
        QStringLiteral("Program Files/Electronic Arts"),
        QStringLiteral("Program Files (x86)/Origin Games"),
        QStringLiteral("Program Files/Epic Games"),
        QStringLiteral("Program Files (x86)/Epic Games"),
        QStringLiteral("Program Files/WindowsApps"),
        QStringLiteral("XboxGames"),
        QStringLiteral("Program Files (x86)/Ubisoft/Ubisoft Game Launcher/games"),
        QStringLiteral("Program Files (x86)/GOG Galaxy/Games"),
        QStringLiteral("GOG Games"),
        QStringLiteral("Amazon Games/Library"),
        QStringLiteral("Riot Games"),
    };
    for (const auto& d : drives) {
        for (const auto& s : suffixes) {
            const QString p = d + s;
            if (QFileInfo::exists(p))
                roots << p;
        }
    }
#else
    // For development on non-Windows we just scan the user's Games folder if any.
    const QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    const QString games = home + QStringLiteral("/Games");
    if (QFileInfo::exists(games))
        roots << games;
#endif
    return roots;
}

void GameScanner::addExtraRoot(const QString& path) {
    if (!path.isEmpty() && !m_extraRoots.contains(path))
        m_extraRoots << path;
}

Platform GameScanner::inferPlatformFromPath(const QString& path) const {
    const QString lower = path.toLower();
    if (lower.contains(QStringLiteral("steam")))                 return Platform::Steam;
    if (lower.contains(QStringLiteral("electronic arts")) ||
        lower.contains(QStringLiteral("ea games")) ||
        lower.contains(QStringLiteral("origin")))                return Platform::EA;
    if (lower.contains(QStringLiteral("epic")))                  return Platform::Epic;
    if (lower.contains(QStringLiteral("windowsapps")) ||
        lower.contains(QStringLiteral("xboxgames")))             return Platform::Microsoft;
    if (lower.contains(QStringLiteral("ubisoft")))               return Platform::Ubisoft;
    if (lower.contains(QStringLiteral("gog")))                   return Platform::GOG;
    if (lower.contains(QStringLiteral("amazon games")))          return Platform::Amazon;
    if (lower.contains(QStringLiteral("riot")))                  return Platform::Riot;
    return Platform::Others;
}

bool GameScanner::looksLikeGameExecutable(const QString& fileName) const {
    const QString lower = fileName.toLower();
    if (!lower.endsWith(QStringLiteral(".exe"))) return false;

    // Skip obvious non-game executables (installers, crash handlers, redistributables).
    static const QStringList blacklist = {
        QStringLiteral("unins"), QStringLiteral("setup"), QStringLiteral("install"),
        QStringLiteral("crash"), QStringLiteral("redist"), QStringLiteral("vcredist"),
        QStringLiteral("dxsetup"), QStringLiteral("launcher_helper"),
        QStringLiteral("dotnet"), QStringLiteral("update"), QStringLiteral("notification"),
    };
    for (const auto& s : blacklist)
        if (lower.contains(s)) return false;

    return true;
}

void GameScanner::scanDirectory(const QString& root, Platform hint, QVector<Game>& out) {
    QDir baseDir(root);
    if (!baseDir.exists()) return;

    // We treat each first-level subfolder as a candidate game install dir.
    const QFileInfoList entries = baseDir.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    for (const QFileInfo& gameDir : entries) {
        // Walk down up to 3 levels deep looking for plausible game .exe files.
        QDirIterator it(gameDir.absoluteFilePath(),
                        QStringList() << QStringLiteral("*.exe"),
                        QDir::Files,
                        QDirIterator::Subdirectories);

        QString bestExe;
        qint64  bestSize = 0;

        while (it.hasNext()) {
            const QString path = it.next();
            const QFileInfo fi(path);
            if (!looksLikeGameExecutable(fi.fileName())) continue;

            // Heuristic: pick the biggest .exe in the install dir.
            if (fi.size() > bestSize) {
                bestSize = fi.size();
                bestExe  = path;
            }
        }

        if (bestExe.isEmpty()) continue;

        Game g;
        g.title          = gameDir.fileName();
        g.installDir     = gameDir.absoluteFilePath();
        g.executablePath = bestExe;
        g.platform       = (hint == Platform::Others) ? inferPlatformFromPath(bestExe) : hint;
        g.id = QString::fromLatin1(
            QCryptographicHash::hash(bestExe.toUtf8(), QCryptographicHash::Sha1).toHex());

        out.push_back(g);
    }
}

#ifdef Q_OS_WIN
QVector<Game> GameScanner::scanRegistry() {
    QVector<Game> result;

    const wchar_t* roots[] = {
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
        L"Software\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
    };
    HKEY hives[] = { HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER };

    for (HKEY hive : hives) {
        for (const wchar_t* root : roots) {
            HKEY h;
            if (RegOpenKeyExW(hive, root, 0, KEY_READ, &h) != ERROR_SUCCESS)
                continue;

            wchar_t subKey[512];
            DWORD   subKeySize = 512;
            DWORD   index = 0;

            while (RegEnumKeyExW(h, index++, subKey, &subKeySize, nullptr,
                                 nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
                subKeySize = 512;

                HKEY sk;
                if (RegOpenKeyExW(h, subKey, 0, KEY_READ, &sk) != ERROR_SUCCESS)
                    continue;

                auto readString = [&](const wchar_t* name) -> QString {
                    wchar_t buf[1024];
                    DWORD   sz = sizeof(buf);
                    DWORD   type = 0;
                    if (RegQueryValueExW(sk, name, nullptr, &type,
                                         reinterpret_cast<LPBYTE>(buf), &sz) == ERROR_SUCCESS) {
                        return QString::fromWCharArray(buf);
                    }
                    return {};
                };

                const QString displayName = readString(L"DisplayName");
                const QString installPath = readString(L"InstallLocation");
                const QString publisher   = readString(L"Publisher");

                RegCloseKey(sk);

                if (displayName.isEmpty() || installPath.isEmpty())
                    continue;

                Game g;
                g.title      = displayName;
                g.installDir = installPath;
                g.platform   = inferPlatformFromPath(installPath);
                if (g.platform == Platform::Others && publisher.toLower().contains("ubisoft"))
                    g.platform = Platform::Ubisoft;
                g.id = QString::fromLatin1(
                    QCryptographicHash::hash(installPath.toUtf8(),
                                             QCryptographicHash::Sha1).toHex());
                result.push_back(g);
            }

            RegCloseKey(h);
        }
    }
    return result;
}
#endif

QVector<Game> GameScanner::scan() {
    QVector<Game> all;
    QSet<QString> seenIds;

    QStringList roots = defaultScanRoots();
    roots += m_extraRoots;

    int idx = 0;
    for (const QString& root : roots) {
        emit scanProgress(idx++, roots.size(), root);
        QVector<Game> found;
        scanDirectory(root, inferPlatformFromPath(root), found);
        for (const Game& g : found) {
            if (!seenIds.contains(g.id)) {
                seenIds.insert(g.id);
                all.push_back(g);
            }
        }
    }

#ifdef Q_OS_WIN
    for (const Game& g : scanRegistry()) {
        if (!seenIds.contains(g.id)) {
            seenIds.insert(g.id);
            all.push_back(g);
        }
    }
#endif

    emit scanFinished(all.size());
    return all;
}

} // namespace omnium
