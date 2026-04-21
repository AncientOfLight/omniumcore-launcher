#pragma once

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "models/Game.h"

namespace omnium {

// Wraps QNetworkAccessManager to query game metadata from RAWG (default) or IGDB.
// Reads the API key from the OMNIUM_RAWG_KEY environment variable.
class ApiClient : public QObject {
    Q_OBJECT
public:
    explicit ApiClient(QObject* parent = nullptr);

    // Async lookup. Emits gameMetadataReady(id, game) on success,
    // or lookupFailed(id, error) on failure.
    void fetchMetadata(const Game& game);

    // Async cover download (kept separate so we can cache it on disk).
    void fetchCover(const QString& gameId, const QUrl& url);

signals:
    void gameMetadataReady(const QString& gameId, const omnium::Game& enriched);
    void coverReady(const QString& gameId, const QString& localPath);
    void lookupFailed(const QString& gameId, const QString& error);

private slots:
    void onMetadataReply(QNetworkReply* reply);

private:
    QNetworkAccessManager m_nam;
    QString               m_rawgKey;
    QString               m_cacheDir;
};

} // namespace omnium
