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
    ~LinkDownloader();

    /// Start the download proces
    void start();

    void downloadFinished(KJob *j);
    void torrentDownloadFinished(KJob *j);

private:
    bool isTorrent(const QByteArray &data) const;
    void handleHtmlPage(const QByteArray &data);
    void tryNextLink();
    void tryTorrentLinks();

Q_SIGNALS:
    void finished(bool ok);

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
};

}

#endif
