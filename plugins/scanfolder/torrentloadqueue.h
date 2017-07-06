/***************************************************************************
 *   Copyright (C) 2011 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef KT_TORRENTLOADQUEUE_H
#define KT_TORRENTLOADQUEUE_H

#include <QUrl>
#include <QTimer>

namespace kt
{

    class CoreInterface;

    /// Action to perform after loading torrent.
    enum LoadedTorrentAction
    {
        DeleteAction,
        MoveAction,
        DefaultAction
    };

    /**
     * Queue of potential torrents. It will try to load them one by one,
     * in a sane and none GUI blocking way.
     */
    class TorrentLoadQueue : public QObject
    {
        Q_OBJECT
    public:
        TorrentLoadQueue(CoreInterface* core, QObject* parent = 0);
        ~TorrentLoadQueue();

        /// Set the loaded torrent action
        void setLoadedTorrentAction(LoadedTorrentAction act) {action = act;}

        /// Get the loaded torrent action
        LoadedTorrentAction loadedTorrentAction() const {return action;}

    public slots:
        /**
         * Add a torrent to load.
         */
        void add(const QUrl& url);

        /**
         * Add a list of torrents
         */
        void add(const QList<QUrl>& urls);

    private:
        /**
         * Validate if a file is a torrent.
         * @param url The file url
         * @param data The torrent data will be put into this array upon success
         * @return true upon success, false otherwise
         */
        bool validateTorrent(const QUrl& url, QByteArray& data);

        /**
         * Load a torrent
         * @param url The file url
         * @param data The torrent data
         */
        void load(const QUrl& url, const QByteArray& data);

    private slots:
        /**
         * Attempt to load one torrent
         */
        void loadOne();

    private:
        /**
         * Loading of a torrent has finished.
         * @param url The url
         */
        void loadingFinished(const QUrl& url);

    private:
        CoreInterface* core;
        QList<QUrl> to_load;
        LoadedTorrentAction action;
        QTimer timer;
    };
}

#endif
