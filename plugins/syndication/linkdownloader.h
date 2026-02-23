/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTLINKDOWNLOADER_H
#define KTLINKDOWNLOADER_H

#include <QObject>
#include <QUrl>

class KJob;

namespace kt
{
class CoreInterface;
struct LinkLoadParams;

/**
    Class to download torrents from links in feeds. Seeing that the links may not directly point to the
    torrent file, some more stuff is needed. If a torrent is found, it is loaded. When it is finished, it will
    commit suicide.
*/
class LinkDownloader : public QObject
{
    Q_OBJECT
public:
    LinkDownloader(const QUrl &url, CoreInterface *core, bool verbose, const QString &group, const QString &location, const QString &move_on_completion);
    LinkDownloader(CoreInterface *core, const LinkLoadParams &params);
    ~LinkDownloader();

    /// Start the download process
    void start();
    /*!
     * Stop and Discard the download
     * The LinkDownloader is not expected to be in a useful state after this
     */
    void kill();

    void downloadFinished(KJob *j);
    void torrentDownloadFinished(KJob *j);

    QUrl getUrl() const;

private:
    bool isTorrent(const QByteArray &data) const;
    void handleHtmlPage(const QByteArray &data);
    void tryNextLink();
    void tryTorrentLinks();
    void registerJob(KJob *job);
    void unregisterJob();

Q_SIGNALS:
    /*!
     * Signal emitted when the LinkDownloader has finished all its tasks
     * It will deleteLater by itself
     */
    void finished(LinkDownloader *downloader, bool ok);
    /*!
     * Signal emitted when the LinkDownloader has downloaded the correct link
     * Any post-download blocking GUI functions will be called afer this
     * This is useful to continue other work while the user is interacting with popups
     */
    void entryDownloadFinished(LinkDownloader *downloader, bool ok);

private:
    QUrl url;
    CoreInterface *core;
    bool verbose;
    QUrl link_url;
    QList<QUrl> links;
    QString group;
    QString location;
    QString move_on_completion;
    QString base_url;
    KJob *startedJob = nullptr;
};

}

#endif
