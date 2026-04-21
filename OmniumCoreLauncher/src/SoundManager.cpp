#include "SoundManager.h"
#include "AssetPaths.h"

namespace omnium {

SoundManager::SoundManager(QObject* parent) : QObject(parent) {
    load(m_click,        QStringLiteral("sounds/click.wav"));
    load(m_launch,       QStringLiteral("sounds/launch.wav"));
    load(m_notification, QStringLiteral("sounds/notification.wav"));
}

void SoundManager::load(QSoundEffect& fx, const QString& relativePath) {
    fx.setSource(AssetPaths::resolveUrl(relativePath));
    fx.setVolume(0.6);
}

void SoundManager::setMuted(bool muted) { m_muted = muted; }

void SoundManager::playClick()        { if (!m_muted) m_click.play(); }
void SoundManager::playLaunch()       { if (!m_muted) m_launch.play(); }
void SoundManager::playNotification() { if (!m_muted) m_notification.play(); }

} // namespace omnium
