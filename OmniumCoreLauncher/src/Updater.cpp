#include "Updater.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrl>

namespace omnium {

Updater::Updater(QObject* parent) : QObject(parent) {}

void Updater::setRepository(const QString& ownerSlashRepo) {
    m_repo = ownerSlashRepo;
}

int Updater::compareVersions(const QString& a, const QString& b) {
    static const QRegularExpression re(QStringLiteral("[^0-9.]"));
    QString ca = a; ca.remove(re);
    QString cb = b; cb.remove(re);
    const QStringList la = ca.split('.', Qt::SkipEmptyParts);
    const QStringList lb = cb.split('.', Qt::SkipEmptyParts);
    const int n = qMax(la.size(), lb.size());
    for (int i = 0; i < n; ++i) {
        const int va = (i < la.size()) ? la[i].toInt() : 0;
        const int vb = (i < lb.size()) ? lb[i].toInt() : 0;
        if (va != vb) return (va < vb) ? -1 : 1;
    }
    return 0;
}

void Updater::checkForUpdates() {
    const QUrl url(QStringLiteral("https://api.github.com/repos/%1/releases/latest")
                   .arg(m_repo));

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QStringLiteral("OmniumCoreLauncher/%1").arg(OMNIUM_VERSION));
    req.setRawHeader("Accept", "application/vnd.github+json");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply* reply = m_nam.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit checkFailed(reply->errorString());
            return;
        }
        const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        const QString tag     = obj.value(QStringLiteral("tag_name")).toString();
        const QString notes   = obj.value(QStringLiteral("body")).toString();

        QString downloadUrl;
        const QJsonArray assets = obj.value(QStringLiteral("assets")).toArray();
        for (const auto& a : assets) {
            const QJsonObject ao = a.toObject();
            const QString name = ao.value(QStringLiteral("name")).toString().toLower();
            if (name.endsWith(QStringLiteral(".exe")) ||
                name.endsWith(QStringLiteral(".msi")) ||
                name.endsWith(QStringLiteral(".zip"))) {
                downloadUrl = ao.value(QStringLiteral("browser_download_url")).toString();
                break;
            }
        }

        const QString current = QStringLiteral(OMNIUM_VERSION);
        if (compareVersions(current, tag) < 0) {
            emit updateAvailable(tag, downloadUrl, notes);
        } else {
            emit upToDate(current);
        }
    });
}

} // namespace omnium
