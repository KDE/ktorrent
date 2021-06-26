/*
    SPDX-FileCopyrightText: 2011 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_TORRENTLOADQUEUE_H
#define KT_TORRENTLOADQUEUE_H

#include <QTimer>
#include <QUrl>

namespace kt
{
class CoreInterface;

/// Action to perform after loading torrent.
enum LoadedTorrentAction {
    DeleteAction,
    MoveAction,
    DefaultAction,
};

/**
 * Queue of potential torrents. It will try to load them one by one,
 * in a sane and none GUI blocking way.
 */
class TorrentLoadQueue : public QObject
{
    Q_OBJECT
public:
    TorrentLoadQueue(CoreInterface *core, QObject *parent = nullptr);
    ~TorrentLoadQueue() override;

    /// Set the loaded torrent action
    void setLoadedTorrentAction(LoadedTorrentAction act)
    {
        action = act;
    }

    /// Get the loaded torrent action
    LoadedTorrentAction loadedTorrentAction() const
    {
        return action;
    }

public Q_SLOTS:
    /**
     * Add a torrent to load.
     */
    void add(const QUrl &url);

    /**
     * Add a list of torrents
     */
    void add(const QList<QUrl> &urls);

private:
    /**
     * Validate if a file is a torrent.
     * @param url The file url
     * @param data The torrent data will be put into this array upon success
     * @return true upon success, false otherwise
     */
    bool validateTorrent(const QUrl &url, QByteArray &data);

    /**
     * Load a torrent
     * @param url The file url
     * @param data The torrent data
     */
    void load(const QUrl &url, const QByteArray &data);

private Q_SLOTS:
    /**
     * Attempt to load one torrent
     */
    void loadOne();

private:
    /**
     * Loading of a torrent has finished.
     * @param url The url
     */
    void loadingFinished(const QUrl &url);

private:
    CoreInterface *core;
    QList<QUrl> to_load;
    LoadedTorrentAction action;
    QTimer timer;
};
}

#endif
