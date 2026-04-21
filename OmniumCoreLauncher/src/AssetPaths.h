#pragma once

#include <QString>
#include <QUrl>

namespace omnium::AssetPaths {

// Returns the first existing path, checking (in order):
//   1. <app_dir>/assets/<relative>
//   2. <app_dir>/../assets/<relative>           (Qt Creator shadow build)
//   3. ":/assets/<relative>"                    (embedded Qt resource)
//
// `relative` is something like "images/logo.svg" or "sounds/click.wav".
QString resolve(const QString& relative);

// Same as resolve(), but always returns a QUrl ready for QSoundEffect /
// QMediaPlayer (uses qrc:/ scheme when the on-disk asset is missing).
QUrl resolveUrl(const QString& relative);

} // namespace omnium::AssetPaths
