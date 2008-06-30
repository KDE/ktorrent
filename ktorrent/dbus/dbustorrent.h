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
#ifndef KTDBUSTORRENT_H
#define KTDBUSTORRENT_H

#include <QObject>
#include <QStringList>
#include <util/constants.h>

namespace bt
{
	class TorrentInterface;
}

namespace kt
{
	class DBusTorrentFile;


	/**
		DBus object to access a torrent
	*/
	class DBusTorrent : public QObject
	{
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "org.ktorrent.torrent")
	public:
		DBusTorrent(bt::TorrentInterface* ti,QObject* parent);
		virtual ~DBusTorrent();
		
		/// Get a pointer to the actual torrent
		bt::TorrentInterface* torrent() {return ti;}
		
	public Q_SLOTS:
		Q_SCRIPTABLE QString infoHash() const;
		Q_SCRIPTABLE QString name() const;
		Q_SCRIPTABLE bool isPrivate() const;
		
		// Speed
		Q_SCRIPTABLE uint downloadSpeed() const;
		Q_SCRIPTABLE uint uploadSpeed() const;
		
		// Data stuff
		Q_SCRIPTABLE qulonglong bytesDownloaded() const;
		Q_SCRIPTABLE qulonglong bytesUploaded() const;
		Q_SCRIPTABLE qulonglong totalSize() const;
		Q_SCRIPTABLE qulonglong bytesLeftToDownload() const;
		Q_SCRIPTABLE qulonglong bytesLeft() const;
		Q_SCRIPTABLE qulonglong bytesToDownload() const;
		
		// Chunks
		Q_SCRIPTABLE uint chunks() const;
		Q_SCRIPTABLE uint chunkSize() const;
		Q_SCRIPTABLE bool chunkDownloaded(uint idx) const;
		
		// Seeders and leechers
		Q_SCRIPTABLE uint seedersConnected() const;
		Q_SCRIPTABLE uint seedersTotal() const;
		Q_SCRIPTABLE uint leechersConnected() const;
		Q_SCRIPTABLE uint leechersTotal() const;
		
		// Tracker stuff
		Q_SCRIPTABLE QString currentTracker() const;
		Q_SCRIPTABLE QStringList trackers() const;
		Q_SCRIPTABLE void setTrackerEnabled(const QString & tracker_url,bool enabled);
		Q_SCRIPTABLE void changeTracker(const QString & tracker_url);
		Q_SCRIPTABLE void announce();
		Q_SCRIPTABLE void scrape();
		Q_SCRIPTABLE bool addTracker(const QString & tracker_url);
		Q_SCRIPTABLE bool removeTracker(const QString & tracker_url);
		Q_SCRIPTABLE void restoreDefaultTrackers();
		
		// WebSeed stuff
		Q_SCRIPTABLE QStringList webSeeds() const;
		Q_SCRIPTABLE bool addWebSeed(const QString & webseed_url);
		Q_SCRIPTABLE bool removeWebSeed(const QString & webseed_url);
		
		// Files
		Q_SCRIPTABLE uint numFiles() const;
		Q_SCRIPTABLE QObject* file(uint idx);
		Q_SCRIPTABLE QString dataDir() const;
		Q_SCRIPTABLE QString torDir() const;
		
		// Stats
		Q_SCRIPTABLE QByteArray stats() const;
		
		
	private:
		bt::TorrentInterface* ti;
		QList<DBusTorrentFile*> files;
	};

}

#endif
