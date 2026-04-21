#include "PlatformIcons.h"

namespace omnium::PlatformIcons {

QIcon iconFor(Platform p) {
    switch (p) {
        case Platform::Steam:     return QIcon(QStringLiteral(":/icons/steam.svg"));
        case Platform::EA:        return QIcon(QStringLiteral(":/icons/ea.svg"));
        case Platform::Epic:      return QIcon(QStringLiteral(":/icons/epic.svg"));
        case Platform::Microsoft: return QIcon(QStringLiteral(":/icons/microsoft.svg"));
        case Platform::Ubisoft:   return QIcon(QStringLiteral(":/icons/ubisoft.svg"));
        case Platform::GOG:       return QIcon(QStringLiteral(":/icons/gog.svg"));
        case Platform::Amazon:    return QIcon(QStringLiteral(":/icons/amazon.svg"));
        case Platform::Riot:      return QIcon(QStringLiteral(":/icons/riot.svg"));
        default:                  return QIcon(QStringLiteral(":/icons/unknown.svg"));
    }
}

QString badgeColorFor(Platform p) {
    switch (p) {
        case Platform::Steam:     return QStringLiteral("#1b2838");
        case Platform::EA:        return QStringLiteral("#ff4747");
        case Platform::Epic:      return QStringLiteral("#2a2a2a");
        case Platform::Microsoft: return QStringLiteral("#107c10");
        case Platform::Ubisoft:   return QStringLiteral("#0070cc");
        case Platform::GOG:       return QStringLiteral("#86328a");
        case Platform::Amazon:    return QStringLiteral("#ff9900");
        case Platform::Riot:      return QStringLiteral("#d13639");
        default:                  return QStringLiteral("#3a3f5a");
    }
}

} // namespace omnium::PlatformIcons
