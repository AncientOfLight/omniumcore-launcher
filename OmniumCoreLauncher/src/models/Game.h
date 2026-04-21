#pragma once

#include <QString>
#include <QStringList>
#include <QDate>
#include <QMetaType>

namespace omnium {

enum class Platform {
    Unknown,
    Steam,
    EA,
    Epic,
    Microsoft,
    Ubisoft,
    GOG,
    Amazon,
    Riot
};

inline QString platformToString(Platform p) {
    switch (p) {
        case Platform::Steam:     return QStringLiteral("Steam");
        case Platform::EA:        return QStringLiteral("EA");
        case Platform::Epic:      return QStringLiteral("Epic");
        case Platform::Microsoft: return QStringLiteral("Microsoft");
        case Platform::Ubisoft:   return QStringLiteral("Ubisoft");
        case Platform::GOG:       return QStringLiteral("GOG");
        case Platform::Amazon:    return QStringLiteral("Amazon");
        case Platform::Riot:      return QStringLiteral("Riot");
        default:                  return QStringLiteral("Unknown");
    }
}

struct Game {
    QString  id;             // Internal id (hash of executablePath)
    QString  title;          // Display name
    QString  executablePath; // Absolute path to .exe on Windows
    QString  installDir;     // Folder where the game lives
    Platform platform = Platform::Unknown;

    // Metadata (filled in by ApiClient — RAWG/IGDB)
    QString     coverUrl;
    QString     coverLocalPath; // Cached cover image path
    QString     synopsis;
    QStringList genres;
    QDate       releaseDate;
    double      rating = 0.0;   // 0..10
};

} // namespace omnium

Q_DECLARE_METATYPE(omnium::Game)
