#pragma once

#include <QObject>
#include <QSoundEffect>

namespace omnium {

// Lightweight wrapper that preloads the three UI sounds and
// exposes named playback methods.
class SoundManager : public QObject {
    Q_OBJECT
public:
    explicit SoundManager(QObject* parent = nullptr);

    void setMuted(bool muted);
    bool isMuted() const { return m_muted; }

public slots:
    void playClick();
    void playLaunch();
    void playNotification();

private:
    void load(QSoundEffect& fx, const QString& relativePath);

    QSoundEffect m_click;
    QSoundEffect m_launch;
    QSoundEffect m_notification;
    bool         m_muted = false;
};

} // namespace omnium
