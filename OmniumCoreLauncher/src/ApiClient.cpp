#include "ApiClient.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QUrl>
#include <QUrlQuery>

namespace omnium {

ApiClient::ApiClient(QObject* parent) : QObject(parent) {
    m_rawgKey  = QString::fromUtf8(qgetenv("OMNIUM_RAWG_KEY"));
    m_cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                 + QStringLiteral("/covers");
    QDir().mkpath(m_cacheDir);

    connect(&m_nam, &QNetworkAccessManager::finished,
            this,  &ApiClient::onMetadataReply);
}

void ApiClient::fetchMetadata(const Game& game) {
    if (m_rawgKey.isEmpty()) {
        emit lookupFailed(game.id, QStringLiteral(
            "Missing RAWG API key — set OMNIUM_RAWG_KEY env var."));
        return;
    }

    QUrl url(QStringLiteral("https://api.rawg.io/api/games"));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("key"), m_rawgKey);
    q.addQueryItem(QStringLiteral("search"), game.title);
    q.addQueryItem(QStringLiteral("page_size"), QStringLiteral("1"));
    url.setQuery(q);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QStringLiteral("OmniumCoreLauncher/%1").arg(OMNIUM_VERSION));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply* reply = m_nam.get(req);
    reply->setProperty("gameId",    game.id);
    reply->setProperty("gameTitle", game.title);
    reply->setProperty("kind",      QStringLiteral("metadata"));
}

void ApiClient::fetchCover(const QString& gameId, const QUrl& url) {
    if (!url.isValid()) return;

    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply* reply = m_nam.get(req);
    reply->setProperty("gameId", gameId);
    reply->setProperty("kind",   QStringLiteral("cover"));
}

void ApiClient::onMetadataReply(QNetworkReply* reply) {
    reply->deleteLater();

    const QString gameId = reply->property("gameId").toString();
    const QString kind   = reply->property("kind").toString();

    if (reply->error() != QNetworkReply::NoError) {
        emit lookupFailed(gameId, reply->errorString());
        return;
    }

    if (kind == QStringLiteral("cover")) {
        const QString path = m_cacheDir + QStringLiteral("/") + gameId
                           + QStringLiteral(".img");
        QFile f(path);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(reply->readAll());
            f.close();
            emit coverReady(gameId, path);
        } else {
            emit lookupFailed(gameId, QStringLiteral("Cannot write cache file."));
        }
        return;
    }

    // Metadata reply (RAWG search results)
    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    const QJsonArray   results = doc.object().value(QStringLiteral("results")).toArray();
    if (results.isEmpty()) {
        emit lookupFailed(gameId, QStringLiteral("No results found."));
        return;
    }

    const QJsonObject first = results.first().toObject();

    Game enriched;
    enriched.id          = gameId;
    enriched.title       = reply->property("gameTitle").toString();
    enriched.coverUrl    = first.value(QStringLiteral("background_image")).toString();
    enriched.synopsis    = first.value(QStringLiteral("description_raw")).toString();
    enriched.releaseDate = QDate::fromString(
        first.value(QStringLiteral("released")).toString(), Qt::ISODate);
    enriched.rating      = first.value(QStringLiteral("rating")).toDouble() * 2.0; // RAWG 0..5 → 0..10

    const QJsonArray genres = first.value(QStringLiteral("genres")).toArray();
    for (const auto& g : genres)
        enriched.genres << g.toObject().value(QStringLiteral("name")).toString();

    emit gameMetadataReady(gameId, enriched);

    if (!enriched.coverUrl.isEmpty())
        fetchCover(gameId, QUrl(enriched.coverUrl));
}

} // namespace omnium
