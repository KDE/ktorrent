/*
    SPDX-FileCopyrightText: 2026 Copilot
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTTORZNABSEARCHJOB_H
#define KTTORZNABSEARCHJOB_H

#include <QDateTime>
#include <QObject>
#include <QQueue>
#include <QUrl>

#include "searchengine.h"

class QNetworkAccessManager;
class QNetworkReply;

namespace kt
{
class ProxyHelper;

struct TorznabSearchResult {
    QString title;
    QString tracker;
    QString link;
    QString descriptionUrl;
    qint64 size = -1;
    int seeders = -1;
    int leechers = -1;
    QDateTime publishedAt;
};

class TorznabSearchJob : public QObject
{
    Q_OBJECT
public:
    TorznabSearchJob(const TorznabEngineConfig &config, const QString &terms, ProxyHelper *proxy, QObject *parent = nullptr);
    ~TorznabSearchJob() override;

    void start();

Q_SIGNALS:
    void resultFound(const kt::TorznabSearchResult &result);
    void fatalError(const QString &message);
    void progressChanged(int completed, int total);
    void finished(int totalIndexers, int failedIndexers);

private Q_SLOTS:
    void indexerListFinished();
    void searchReplyFinished();

private:
    QUrl buildJackettUrl(const QString &relativePath) const;
    QUrl buildIndexerListUrl() const;
    QUrl buildIndexerSearchUrl(const QString &indexerId) const;
    QStringList parseIndexers(const QByteArray &data) const;
    bool parseResults(const QByteArray &data, const QString &defaultTracker, QList<TorznabSearchResult> *results) const;
    void startNextRequests();
    void finishIfDone();
    void fail(const QString &message);

    QNetworkAccessManager *manager;
    TorznabEngineConfig config;
    QString terms;
    ProxyHelper *proxy;
    QQueue<QString> pendingIndexers;
    int activeRequests = 0;
    int completedRequests = 0;
    int totalIndexers = 0;
    int failedIndexers = 0;
    bool finishedEmitted = false;
};
}

Q_DECLARE_METATYPE(kt::TorznabSearchResult)

#endif
