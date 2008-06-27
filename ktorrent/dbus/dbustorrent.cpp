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
#include <QDBusConnection>
#include <util/sha1hash.h>
#include <util/bitset.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/trackerslist.h>
#include <interfaces/webseedinterface.h>
#include "dbustorrent.h"
#include "dbustorrentfile.h"

using namespace bt;

namespace kt
{

	DBusTorrent::DBusTorrent(int id,bt::TorrentInterface* ti,QObject* parent)
			: QObject(parent),dbus_id(id),ti(ti)
	{
		QDBusConnection sb = QDBusConnection::sessionBus();
		QString path = QString("/torrent/%1").arg(id);
		QFlags<QDBusConnection::RegisterOption> flags = QDBusConnection::ExportScriptableSlots|QDBusConnection::ExportScriptableSignals;
		sb.registerObject(path, this,flags);
		
		for (uint i = 0;i < ti->getNumFiles();i++)
		{
			DBusTorrentFile* tf = new DBusTorrentFile(ti->getTorrentFile(i),this);
			sb.registerObject(path + QString("/file/%1").arg(i),tf,flags);
		}
	}


	DBusTorrent::~DBusTorrent()
	{
	}

	QString DBusTorrent::infoHash() const
	{
		const bt::SHA1Hash & h = ti->getInfoHash();
		return h.toString();
	}
	
	QString DBusTorrent::name() const
	{
		return ti->getStats().torrent_name;
	}
	
	bool DBusTorrent::isPrivate() const
	{
		return ti->getStats().priv_torrent;
	}
	
	uint DBusTorrent::downloadSpeed() const
	{
		return ti->getStats().download_rate;
	}
	
	uint DBusTorrent::uploadSpeed() const
	{
		return ti->getStats().upload_rate;
	}
	
	qulonglong DBusTorrent::bytesDownloaded() const
	{
		return ti->getStats().bytes_downloaded;
	}
	
	qulonglong DBusTorrent::bytesUploaded() const
	{
		return ti->getStats().bytes_uploaded;
	}
	
	qulonglong DBusTorrent::totalSize() const
	{
		return ti->getStats().total_bytes;
	}
	
	qulonglong DBusTorrent::bytesLeftToDownload() const
	{
		return ti->getStats().bytes_left_to_download;
	}
	
	qulonglong DBusTorrent::bytesLeft() const
	{
		return ti->getStats().bytes_left;
	}
	
	qulonglong DBusTorrent::bytesToDownload() const
	{
		return ti->getStats().total_bytes_to_download;
	}
	
	uint DBusTorrent::chunks() const
	{
		return ti->getStats().total_chunks;
	}
	
	uint DBusTorrent::chunkSize() const
	{
		return ti->getStats().chunk_size;
	}
	
	bool DBusTorrent::chunkDownloaded(uint idx) const
	{
		return ti->downloadedChunksBitSet().get(idx);
	}

	uint DBusTorrent::seedersConnected() const
	{
		return ti->getStats().seeders_connected_to;
	}
	
	uint DBusTorrent::seedersTotal() const
	{
		return ti->getStats().seeders_total;
	}
	
	uint DBusTorrent::leechersConnected() const
	{
		return ti->getStats().leechers_connected_to;
	}
	
	uint DBusTorrent::leechersTotal() const
	{
		return ti->getStats().leechers_total;
	}
	
	QString DBusTorrent::currentTracker() const
	{
		return ti->getTrackersList()->getTrackerURL().prettyUrl();
	}
	
	QStringList DBusTorrent::trackers() const
	{
		TrackersList* tlist = ti->getTrackersList();
		KUrl::List urls = tlist->getTrackerURLs();
		QStringList ret;
		foreach (KUrl u,urls)
			ret << u.prettyUrl();
		return ret;
	}
	
	void DBusTorrent::changeTracker(const QString & tracker_url)
	{
		KUrl url(tracker_url);
		ti->getTrackersList()->setTracker(url);
	}
	
	void DBusTorrent::announce()
	{
		ti->updateTracker();
	}
	
	void DBusTorrent::scrape()
	{
		ti->scrapeTracker();
	}
	
	void DBusTorrent::setTrackerEnabled(const QString & tracker_url,bool enabled)
	{
		ti->getTrackersList()->setTrackerEnabled(KUrl(tracker_url),enabled);
	}
	
	bool DBusTorrent::addTracker(const QString & tracker_url)
	{
		if (ti->getStats().priv_torrent)
			return false;
		
		ti->getTrackersList()->addTracker(KUrl(tracker_url),true);
		return true;
	}
	
	bool DBusTorrent::removeTracker(const QString & tracker_url)
	{
		if (ti->getStats().priv_torrent)
			return false;
		
		ti->getTrackersList()->removeTracker(KUrl(tracker_url));
		return true;
	}
	
	void DBusTorrent::restoreDefaultTrackers()
	{
		ti->getTrackersList()->restoreDefault();
		ti->updateTracker();
	}
	
	QStringList DBusTorrent::webSeeds() const
	{
		QStringList ws;
		for (Uint32 i = 0;i < ti->getNumWebSeeds();i++)
		{
			const WebSeedInterface* wsi = ti->getWebSeed(i);
			ws << wsi->getUrl().prettyUrl();
		}
		return ws;
	}
	
	bool DBusTorrent::addWebSeed(const QString & webseed_url)
	{
		return ti->addWebSeed(KUrl(webseed_url));
	}
			
	bool DBusTorrent::removeWebSeed(const QString & webseed_url)
	{
		return ti->removeWebSeed(KUrl(webseed_url));
	}
	
	uint DBusTorrent::numFiles() const
	{
		return ti->getNumFiles();
	}
	
	QString DBusTorrent::dataDir() const
	{
		return ti->getDataDir();
	}
	
	QString DBusTorrent::torDir() const
	{
		return ti->getTorDir();
	}
}
