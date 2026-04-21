#pragma once

#include <QIcon>
#include <QString>
#include "models/Game.h"

namespace omnium::PlatformIcons {

QIcon   iconFor(Platform p);
QString badgeColorFor(Platform p); // Hex color used in QSS for badges

} // namespace omnium::PlatformIcons
