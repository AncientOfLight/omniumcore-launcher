#pragma once

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>

namespace omnium {

// Compares the locally compiled OMNIUM_VERSION against the latest GitHub release tag.
// Configure the repo via setRepository("owner/repo").
class Updater : public QObject {
    Q_OBJECT
public:
    explicit Updater(QObject* parent = nullptr);

    void setRepository(const QString& ownerSlashRepo);

public slots:
    // Triggers an async check. Emits one of the signals below.
    void checkForUpdates();

signals:
    void updateAvailable(const QString& latestVersion,
                         const QString& downloadUrl,
                         const QString& releaseNotes);
    void upToDate(const QString& currentVersion);
    void checkFailed(const QString& error);

private:
    static int compareVersions(const QString& a, const QString& b);

    QNetworkAccessManager m_nam;
    QString               m_repo = QStringLiteral("OmniumCore/launcher");
};

} // namespace omnium
