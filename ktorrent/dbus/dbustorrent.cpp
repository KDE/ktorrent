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
#include <bcodec/bencoder.h>
#include "dbustorrent.h"
#include "dbustorrentfile.h"

using namespace bt;

namespace kt
{

	DBusTorrent::DBusTorrent(bt::TorrentInterface* ti,QObject* parent)
			: QObject(parent),ti(ti)
	{
		QDBusConnection sb = QDBusConnection::sessionBus();
		QString path = QString("/torrent/%1").arg(ti->getInfoHash().toString());
		QFlags<QDBusConnection::RegisterOption> flags = QDBusConnection::ExportScriptableSlots|QDBusConnection::ExportScriptableSignals;
		sb.registerObject(path, this,flags);
		
		for (uint i = 0;i < ti->getNumFiles();i++)
		{
			DBusTorrentFile* tf = new DBusTorrentFile(ti->getTorrentFile(i),this);
			sb.registerObject(path + QString("/file/%1").arg(i),tf,flags);
			files.append(tf);
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
		return ti->getDisplayName();
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
		foreach (const KUrl& u,urls)
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
	
	QObject* DBusTorrent::file(uint idx)
	{
		if (idx >= files.count())
			return 0;
		else
			return files.at(idx);
	}
	
	QString DBusTorrent::dataDir() const
	{
		return ti->getDataDir();
	}
	
	QString DBusTorrent::torDir() const
	{
		return ti->getTorDir();
	}
	
	QByteArray DBusTorrent::stats() const
	{
		QByteArray ret;
		BEncoder enc(new BEncoderBufferOutput(ret));
		const TorrentStats & s = ti->getStats();
		enc.beginDict();
		enc.write("imported_bytes",s.imported_bytes);
		enc.write("bytes_downloaded",s.bytes_downloaded);
		enc.write("bytes_uploaded",s.bytes_uploaded);
		enc.write("bytes_left",s.bytes_left);
		enc.write("bytes_left_to_download",s.bytes_left_to_download);
		enc.write("total_bytes",s.total_bytes);
		enc.write("total_bytes_to_download",s.total_bytes_to_download);
		enc.write("download_rate",s.download_rate);
		enc.write("upload_rate",s.upload_rate);
		enc.write("num_peers",s.num_peers);
		enc.write("num_chunks_downloading",s.num_chunks_downloading);
		enc.write("total_chunks",s.total_chunks);
		enc.write("num_chunks_downloaded",s.num_chunks_downloaded);
		enc.write("num_chunks_excluded",s.num_chunks_excluded);
		enc.write("num_chunks_left",s.num_chunks_left);
		enc.write("chunk_size",s.chunk_size);
		enc.write("seeders_total",s.seeders_total);
		enc.write("seeders_connected_to",s.seeders_connected_to);
		enc.write("leechers_total",s.leechers_total);
		enc.write("leechers_connected_to",s.leechers_connected_to);
		enc.write("total_times_downloaded", s.total_times_downloaded);
		enc.write("status",ti->statusToString());
		enc.write("tracker_status", s.trackerstatus);
		enc.write("session_bytes_downloaded", s.session_bytes_downloaded);
		enc.write("session_bytes_uploaded", s.session_bytes_uploaded);
		enc.write("trk_bytes_downloaded", s.trk_bytes_downloaded);
		enc.write("trk_bytes_uploaded", s.trk_bytes_uploaded);
		enc.write("output_path", s.output_path);
		enc.write("running",s.running);
		enc.write("started", s.started);
		enc.write("multi_file_torrent", s.multi_file_torrent);
		enc.write("stopped_by_error", s.stopped_by_error);
		enc.write("completed", s.completed);
		enc.write("user_controlled", s.user_controlled);
		enc.write("max_share_ratio", s.max_share_ratio);
		enc.write("max_seed_time", s.max_seed_time);
		enc.write("num_corrupted_chunks", s.num_corrupted_chunks);
		const bt::BitSet & bs = ti->downloadedChunksBitSet();
		enc.write("downloaded_chunks");
		enc.write(bs.getData(),bs.getNumBytes());
		const bt::BitSet & ebs = ti->excludedChunksBitSet();
		enc.write("excluded_chunks");
		enc.write(ebs.getData(),ebs.getNumBytes());
		enc.end();
		return ret;
	}
}
