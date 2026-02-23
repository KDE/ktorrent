/*
    SPDX-FileCopyrightText: 2025 Aditya Tolikar <ulterno@proton.me>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DOWNLOADQUEUE_H
#define DOWNLOADQUEUE_H

#include <QMultiMap>
#include <QObject>
#include <QPointer>
#include <QUrl>
#include <interfaces/coreinterface.h>

namespace kt
{

class LinkDownloader;
class SyndicationActivity;
class SyndicationPlugin;
class Feed;
struct LinkLoadParams;

class DownloadQueue : public QObject
{
    Q_OBJECT
public:
    explicit DownloadQueue(SyndicationPlugin *plugin, QObject *parent = nullptr);
    ~DownloadQueue();

    void downloadLink(const QUrl &url, const QString &group, const QString &location, const QString &move_on_completion, bool silently, Feed *sender);
    // ToDo: make function void continueToNextDownload(); usable through UI
private:
    struct QueuedDownload {
        LinkLoadParams link_params;
        QPointer<Feed> feed;
        bool created = false;
        LinkDownloader *downloader = nullptr;
    };

    QMap<QString, QList<QueuedDownload>> download_queues;

    /// Initialise LinkDownloader for Feed Items and make corresponding connections
    void createLinkDownloader(QueuedDownload &download_entry);

    /// Continue download of next entry in the queue of the given hostname
    void continueToNextDownload(const QString &hostname);

    void handleLinkDownloadFinished(LinkDownloader *downloader, bool success);

    SyndicationPlugin *m_plugin;
};
}
#endif // DOWNLOADQUEUE_H
