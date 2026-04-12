/*
    SPDX-FileCopyrightText: 2026 Copilot
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "torznabsearchjob.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QXmlStreamAttributes>
#include <QXmlStreamReader>

#include "proxy_helper.h"

namespace kt
{
TorznabSearchJob::TorznabSearchJob(const TorznabEngineConfig &config, const QString &terms, ProxyHelper *proxy, QObject *parent)
    : QObject(parent)
    , manager(new QNetworkAccessManager(this))
    , config(config)
    , terms(terms)
    , proxy(proxy)
{
    if (this->proxy) {
        this->proxy->applyNetworkProxy(manager);
    }
}

TorznabSearchJob::~TorznabSearchJob()
{
}

void TorznabSearchJob::start()
{
    if (!config.serviceUrl.isValid() || config.serviceUrl.host().isEmpty()) {
        fail(tr("The configured Jackett/Torznab URL is invalid."));
        return;
    }

    if (config.apiKey.trimmed().isEmpty()) {
        fail(tr("The configured Jackett/Torznab API key is empty."));
        return;
    }

    QNetworkReply *reply = manager->get(QNetworkRequest(buildIndexerListUrl()));
    connect(reply, &QNetworkReply::finished, this, &TorznabSearchJob::indexerListFinished);
}

QUrl TorznabSearchJob::buildJackettUrl(const QString &relativePath) const
{
    QUrl url(config.serviceUrl);
    QString path = url.path();
    if (!path.endsWith(QLatin1Char('/'))) {
        path += QLatin1Char('/');
    }
    path += relativePath;
    url.setPath(path);
    return url;
}

QUrl TorznabSearchJob::buildIndexerListUrl() const
{
    QUrl url = buildJackettUrl(QStringLiteral("api/v2.0/indexers/all/results/torznab/api"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("apikey"), config.apiKey);
    query.addQueryItem(QStringLiteral("t"), QStringLiteral("indexers"));
    query.addQueryItem(QStringLiteral("configured"), QStringLiteral("true"));
    url.setQuery(query);
    return url;
}

QUrl TorznabSearchJob::buildIndexerSearchUrl(const QString &indexerId) const
{
    QUrl url = buildJackettUrl(QStringLiteral("api/v2.0/indexers/%1/results/torznab/api").arg(indexerId));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("apikey"), config.apiKey);
    query.addQueryItem(QStringLiteral("q"), terms);
    url.setQuery(query);
    return url;
}

QStringList TorznabSearchJob::parseIndexers(const QByteArray &data) const
{
    QStringList indexers;
    QXmlStreamReader reader(data);

    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement() && reader.name() == QLatin1String("indexer")) {
            const QString id = reader.attributes().value(QLatin1String("id")).toString().trimmed();
            if (!id.isEmpty()) {
                indexers.append(id);
            }
        }
    }

    if (reader.hasError()) {
        return {};
    }

    return indexers;
}

bool TorznabSearchJob::parseResults(const QByteArray &data, const QString &defaultTracker, QList<TorznabSearchResult> *results) const
{
    QXmlStreamReader reader(data);
    TorznabSearchResult current;
    bool inItem = false;

    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement()) {
            const QStringView name = reader.name();
            if (name == QLatin1String("item")) {
                current = TorznabSearchResult();
                current.tracker = defaultTracker;
                inItem = true;
            } else if (!inItem) {
                continue;
            } else if (name == QLatin1String("title")) {
                current.title = reader.readElementText().trimmed();
            } else if (name == QLatin1String("link")) {
                current.link = reader.readElementText().trimmed();
            } else if (name == QLatin1String("comments")) {
                current.descriptionUrl = reader.readElementText().trimmed();
            } else if (name == QLatin1String("guid")) {
                const QString guid = reader.readElementText().trimmed();
                if (current.descriptionUrl.isEmpty()) {
                    current.descriptionUrl = guid;
                }
            } else if (name == QLatin1String("size")) {
                bool ok = false;
                const qint64 size = reader.readElementText().trimmed().toLongLong(&ok);
                current.size = ok ? size : -1;
            } else if (name == QLatin1String("pubDate")) {
                current.publishedAt = QDateTime::fromString(reader.readElementText().trimmed(), Qt::RFC2822Date);
            } else if (name == QLatin1String("jackettindexer")) {
                const QString tracker = reader.readElementText().trimmed();
                if (!tracker.isEmpty()) {
                    current.tracker = tracker;
                }
            } else if (name == QLatin1String("attr")) {
                const QXmlStreamAttributes attributes = reader.attributes();
                const QString attrName = attributes.value(QLatin1String("name")).toString();
                const QString attrValue = attributes.value(QLatin1String("value")).toString();

                if (attrName == QLatin1String("magneturl") && !attrValue.isEmpty()) {
                    current.link = attrValue;
                } else if (attrName == QLatin1String("seeders")) {
                    current.seeders = attrValue.toInt();
                } else if (attrName == QLatin1String("peers")) {
                    current.leechers = attrValue.toInt();
                }
            }
        } else if (reader.isEndElement() && reader.name() == QLatin1String("item")) {
            if (!current.title.isEmpty() && !current.link.isEmpty()) {
                if (current.seeders >= 0 && current.leechers >= current.seeders) {
                    current.leechers -= current.seeders;
                }
                results->append(current);
            }

            inItem = false;
        }
    }

    return !reader.hasError();
}

void TorznabSearchJob::indexerListFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        fail(tr("Failed to retrieve the configured Jackett indexers."));
        return;
    }

    const QByteArray payload = reply->readAll();
    const bool hasError = reply->error() != QNetworkReply::NoError;
    reply->deleteLater();

    if (hasError) {
        fail(tr("Failed to retrieve the configured Jackett indexers."));
        return;
    }

    const QStringList indexers = parseIndexers(payload);
    if (indexers.isEmpty()) {
        fail(tr("No configured Jackett indexers were returned by the server."));
        return;
    }

    totalIndexers = indexers.size();
    pendingIndexers.clear();
    for (const QString &indexer : indexers) {
        pendingIndexers.enqueue(indexer);
    }
    Q_EMIT progressChanged(0, totalIndexers);
    startNextRequests();
}

void TorznabSearchJob::searchReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    const QString tracker = reply->property("indexerId").toString();
    const QByteArray payload = reply->readAll();
    const bool hasError = reply->error() != QNetworkReply::NoError;
    reply->deleteLater();

    activeRequests = qMax(0, activeRequests - 1);
    completedRequests++;

    if (hasError) {
        failedIndexers++;
    } else {
        QList<TorznabSearchResult> parsedResults;
        if (!parseResults(payload, tracker, &parsedResults)) {
            failedIndexers++;
        } else {
            for (const TorznabSearchResult &result : parsedResults) {
                Q_EMIT resultFound(result);
            }
        }
    }

    Q_EMIT progressChanged(completedRequests, totalIndexers);
    startNextRequests();
    finishIfDone();
}

void TorznabSearchJob::startNextRequests()
{
    const int maxConcurrent = qMax(1, config.threadCount);
    while (activeRequests < maxConcurrent && !pendingIndexers.isEmpty()) {
        const QString indexerId = pendingIndexers.dequeue();
        QNetworkReply *reply = manager->get(QNetworkRequest(buildIndexerSearchUrl(indexerId)));
        reply->setProperty("indexerId", indexerId);
        connect(reply, &QNetworkReply::finished, this, &TorznabSearchJob::searchReplyFinished);
        activeRequests++;
    }

    finishIfDone();
}

void TorznabSearchJob::finishIfDone()
{
    if (!finishedEmitted && totalIndexers > 0 && activeRequests == 0 && pendingIndexers.isEmpty()) {
        finishedEmitted = true;
        Q_EMIT finished(totalIndexers, failedIndexers);
    }
}

void TorznabSearchJob::fail(const QString &message)
{
    if (finishedEmitted) {
        return;
    }

    finishedEmitted = true;
    Q_EMIT fatalError(message);
    Q_EMIT finished(0, 0);
}
}
