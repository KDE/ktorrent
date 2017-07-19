/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
        LinkDownloader(const QUrl& url, CoreInterface* core, bool verbose, const QString& group, const QString& location, const QString& move_on_completion);
        ~LinkDownloader();

        /// Start the download proces
        void start();

        void downloadFinished(KJob* j);
        void torrentDownloadFinished(KJob* j);

    private:
        bool isTorrent(const QByteArray& data) const;
        void handleHtmlPage(const QByteArray& data);
        void tryNextLink();
        void tryTorrentLinks();

    Q_SIGNALS:
        void finished(bool ok);

    private:
        QUrl url;
        CoreInterface* core;
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
