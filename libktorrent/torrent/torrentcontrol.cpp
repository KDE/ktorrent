/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Ivan Vasic <ivasic@gmail.com>                                         *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <qfile.h>
#include <klocale.h>
#include <qtextstream.h>
#include <util/fileops.h>
#include "downloader.h"
#include "uploader.h"
#include "tracker.h"
#include "chunkmanager.h"
#include "torrent.h"
#include "peermanager.h"
#include <util/error.h>
#include <util/log.h>
#include <util/functions.h>
#include "torrentfile.h"
#include "torrentcontrol.h"
#include "bitset.h"
#include "peer.h"
#include "choker.h"

#include "globals.h"
#include "server.h"
#include "packetwriter.h"
#include "httptracker.h"
#include "udptracker.h"
#include "downloadcap.h"
#include "uploadcap.h"

using namespace kt;

namespace bt
{


	TorrentControl::TorrentControl()
			: tor(0),tracker(0),cman(0),pman(0),down(0),up(0),choke(0),tmon(0)
	{
		stats.running = false;
		stats.started = false;
		stats.saved = false;
		stats.stopped_by_error = false;
		stats.session_bytes_downloaded = 0;
		stats.session_bytes_uploaded = 0;
		
		
		old_datadir = QString::null;
		tracker_update_interval = 120000;
		stats.status = NOT_STARTED;
		stats.autostart = true;
		running_time_dl = running_time_ul = 0;
		prev_bytes_dl = 0;
		prev_bytes_ul = 0;

		updateStats();
	}




	TorrentControl::~TorrentControl()
	{
		if (stats.running)
			stop(false);

		if (tmon)
			tmon->destroyed();

		delete down;
		delete up;
		delete cman;
		delete pman;
		delete tracker;
		delete tor;
	}

	void TorrentControl::update()
	{
		// first update peermanager
		try
		{
			pman->update();
			bool comp = stats.completed;

			// then the downloader and uploader
			up->update(choke->getOptimisticlyUnchokedPeerID());
			//if (!completed)
			down->update();

			stats.completed = cman->chunksLeft() == 0;
			if (stats.completed && !comp)
			{
				// download has just been completed
				updateTracker("completed");
				finished(this);
				pman->killSeeders();
				QDateTime now = QDateTime::currentDateTime();
				running_time_dl += time_started_dl.secsTo(now);
			}
			else if (!stats.completed && comp)
			{
				// restart download if necesarry
				// when user selects that files which were previously excluded,
				// should now be downloaded
				updateTracker("started");
				time_started_dl = QDateTime::currentDateTime();
			}
			updateStatusMsg();

			// make sure we don't use up to much memory with all the chunks
			// we're uploading
			cman->checkMemoryUsage();

			// get rid of dead Peers
			pman->clearDeadPeers();

			// we may need to update the tracker
			if (tracker_update_timer.getElapsedSinceUpdate() >= tracker_update_interval)
			{
				Uint32 max_connections = PeerManager::getMaxConnections();
				// if a peer has nothing but choked since the 5 last tracker updates
				// we will get rid of them (if there is a connection cap)
				if (max_connections > 0 && pman->getNumConnectedPeers() == max_connections)
					pman->killChokedPeers(5*tracker_update_interval);

				updateTracker();
				tracker_update_timer.update();
			}

			// we may need to update the choker
			if (choker_update_timer.getElapsedSinceUpdate() >= 10000)
			{
				// also get rid of seeders when download is finished
				// no need to keep them around, but also no need to do this
				// every update, so once every 10 seconds is fine
				if (stats.completed)
					pman->killSeeders();
				
				doChoking();
				choker_update_timer.update();
			}

			// to satisfy people obsessed with their share ratio
			if (stats_save_timer.getElapsedSinceUpdate() >= 5*60*1000)
			{
				saveStats();
				stats_save_timer.update();
			}

			// Update DownloadCap
			DownloadCap::instance().update();
			UploadCap::instance().update();
			updateStats();
		}
		catch (Error & e)
		{
			Out() << "Error : " << e.toString() << endl;
			stats.stopped_by_error = true;
			error_msg = e.toString();
			short_error_msg = e.toString();
			stop(false);
			emit stoppedByError(this, error_msg);
		}
	}

	void TorrentControl::start()
	{
		if (bt::Exists(datadir + "stopped"))
			bt::Delete(datadir + "stopped",true);

		stats.stopped_by_error = false;
		updateTracker("started");
		pman->start();
		down->loadDownloads(datadir + "current_chunks");
		loadStats();
		stats.running = true;
		stats.started = true;
		tracker_update_timer.update();
		choker_update_timer.update();
		stats_save_timer.update();
		time_started_ul = time_started_dl = QDateTime::currentDateTime();
	}

	void TorrentControl::stop(bool user)
	{
		QDateTime now = QDateTime::currentDateTime();
		if(!stats.completed)
			running_time_dl += time_started_dl.secsTo(now);
		running_time_ul += time_started_ul.secsTo(now);
		time_started_ul = time_started_dl = now;
	
		if (stats.running)
		{
			updateTracker("stopped");

			if (tmon)
				tmon->stopped();

			down->saveDownloads(datadir + "current_chunks");
			down->clearDownloads();
			if (user)
				bt::Touch(datadir + "stopped",true);
		}
		pman->stop();
		pman->closeAllConnections();
		pman->clearDeadPeers();

		stats.running = false;
		saveStats();
		updateStatusMsg();
	}

	void TorrentControl::setMonitor(kt::MonitorInterface* tmo)
	{
		tmon = tmo;
		down->setMonitor(tmon);
		if (tmon)
		{
			for (Uint32 i = 0;i < pman->getNumConnectedPeers();i++)
				tmon->peerAdded(pman->getPeer(i));
		}
	}

	void TorrentControl::init(const QString & torrent,const QString & ddir)
	{
		datadir = ddir;
		stats.completed = false;
		stats.running = false;
		if (!datadir.endsWith(DirSeparator()))
			datadir += DirSeparator();


		// first load the torrent file
		tor = new Torrent();
		tor->load(torrent,false);
		if (!bt::Exists(datadir))
			bt::MakeDir(datadir);

		stats.torrent_name = tor->getNameSuggestion();
		stats.multi_file_torrent = tor->isMultiFile();
		stats.total_bytes = tor->getFileLength();
		
		// copy torrent in temp dir
		QString tor_copy = datadir + "torrent";

		if (tor_copy != torrent)
			bt::CopyFile(torrent,tor_copy);


		// create PeerManager and Tracker
		pman = new PeerManager(*tor);
		if (tor->getTrackerURL(true).protocol() == "udp")
			tracker = new UDPTracker();
		else
			tracker = new HTTPTracker();

		connect(tracker,SIGNAL(error()),this,SLOT(trackerResponseError()));
		connect(tracker,SIGNAL(dataReady()),this,SLOT(trackerResponse()));

		// Create chunkmanager, load the index file if it exists
		// else create all the necesarry files
		cman = new ChunkManager(*tor,datadir);
		if (bt::Exists(datadir + "index"))
			cman->loadIndexFile();
		else
			cman->createFiles();

		stats.completed = cman->chunksLeft() == 0;

		// create downloader,uploader and choker
		down = new Downloader(*tor,*pman,*cman);
		up = new Uploader(*cman,*pman);
		choke = new Choker(*pman);


		connect(pman,SIGNAL(newPeer(Peer* )),this,SLOT(onNewPeer(Peer* )));
		connect(pman,SIGNAL(peerKilled(Peer* )),this,SLOT(onPeerRemoved(Peer* )));
		connect(cman,SIGNAL(excluded(Uint32, Uint32 )),
		        down,SLOT(onExcluded(Uint32, Uint32 )));

		stats.saved = cman->hasBeenSaved();
		if (bt::Exists(datadir + "stopped"))
			stats.autostart = false;
		updateStatusMsg();

		// to get rid of phantom bytes we need to take into account
		// the data from downloads allready in progress
		down->loadDownloads(datadir + "current_chunks");
		prev_bytes_dl = down->bytesDownloaded();
		down->clearDownloads();

		loadStats();
	}

	void TorrentControl::setTrackerTimerInterval(Uint32 interval)
	{
		tracker_update_interval = interval;
	}

	void TorrentControl::trackerResponse()
	{
		try
		{
			tracker->updateData(this,pman);
			updateStatusMsg();
			stats.trackerstatus = i18n("OK");
		}
		catch (Error & e)
		{
			Out() << "Error : " << e.toString() << endl;
			if (trackerevent != "stopped")
			{
				stats.trackerstatus = i18n("Invalid response");
				updateTracker(trackerevent,false);
			}
		}
	}

	void TorrentControl::trackerResponseError()
	{
		Out() << "Tracker Response Error" << endl;
		if (trackerevent != "stopped")
		{
			stats.trackerstatus = i18n("Unreachable");
			updateTracker(trackerevent,false);
		}
	}

	void TorrentControl::updateTracker(const QString & ev,bool last_succes)
	{
		trackerevent = ev;
		if (!tor || !tracker || !down || !up)
			return;

		KURL url = tor->getTrackerURL(last_succes);
		last_tracker_url = url;
		port = Globals::instance().getServer().getPortInUse();
		tracker->setData(tor->getInfoHash(),tor->getPeerID(),port,
		                 up->bytesUploaded(),down->bytesDownloaded(),
		                 cman->bytesLeft(),ev);

		tracker->doRequest(url);
	}


	void TorrentControl::onNewPeer(Peer* p)
	{
		p->getPacketWriter().sendBitSet(cman->getBitSet());
		if (tmon)
			tmon->peerAdded(p);
	}

	void TorrentControl::onPeerRemoved(Peer* p)
	{
		if (tmon)
			tmon->peerRemoved(p);
	}

	void TorrentControl::doChoking()
	{
		choke->update(cman->bytesLeft() == 0);
	}
	
	void TorrentControl::reconstruct(const QString & dir)
	{
		if (stats.saved && !tor->isMultiFile())
			return;

		cman->save(dir);
		stats.saved = true;
	}

	

	bool TorrentControl::changeDataDir(const QString & new_dir)
	{
		// new_dir doesn't contain the torX/ part
		// so first get that and append it to new_dir
		int dd = datadir.findRev(DirSeparator(),datadir.length() - 2,false);
		QString tor = datadir.mid(dd + 1,datadir.length() - 2 - dd);


		// make sure nd ends with a /
		QString nd = new_dir + tor;
		if (!nd.endsWith(DirSeparator()))
			nd += DirSeparator();

		Out() << datadir << " -> " << nd << endl;

		int ok_calls = 0;
		try
		{
			if (!bt::Exists(nd))
				bt::MakeDir(nd);

			// now try to move all the files :
			// first the torrent
			bt::Move(datadir + "torrent",nd);
			ok_calls++;
			// then the index
			bt::Move(datadir + "index",nd);
			ok_calls++;
			// then the cache
			bt::Move(datadir + "cache",nd);
			ok_calls++;

			// tell the chunkmanager that the datadir has changed
			cman->changeDataDir(nd);
		}
		catch (...)
		{
			// move the torrent back
			if (ok_calls >= 1)
				bt::Move(nd + "torrent",datadir,true);
			if (ok_calls >= 2)
				bt::Move(nd + "index",datadir,true);
			return false;
		}

		// we don't move the current_chunks file
		// it will be recreated anyway
		// now delete the old directory
		bt::Delete(datadir,true);

		old_datadir = datadir;
		datadir = nd;
		return true;
	}


	void TorrentControl::rollback()
	{
		if (old_datadir.isNull())
			return;

		// recreate it
		if (!bt::Exists(old_datadir))
			bt::MakeDir(old_datadir,true);

		// move back files
		bt::Move(datadir + "torrent",old_datadir,true);
		bt::Move(datadir + "cache",old_datadir,true);
		bt::Move(datadir + "index",old_datadir,true);
		cman->changeDataDir(old_datadir);

		// delete new
		bt::Delete(datadir,true);

		datadir = old_datadir;
		old_datadir = QString::null;
	}

	void TorrentControl::updateStatusMsg()
	{
		if (stats.stopped_by_error)
			stats.status = kt::ERROR;
		else if (!stats.started)
			stats.status = kt::NOT_STARTED;
		else if (!stats.running && stats.completed)
			stats.status = kt::COMPLETE;
		else if (!stats.running)
			stats.status = kt::STOPPED;
		else if (stats.running && stats.completed)
			stats.status = kt::SEEDING;
		else if (stats.running)
			stats.status = down->downloadRate() > 0 ?
					kt::DOWNLOADING : kt::STALLED;
	}

	const BitSet & TorrentControl::downloadedChunksBitSet() const
	{
		if (cman)
			return cman->getBitSet();
		else
			return BitSet::null;
	}

	const BitSet & TorrentControl::availableChunksBitSet() const
	{
		if (!pman)
			return BitSet::null;
		else
			return pman->getAvailableChunksBitSet();
	}

	const BitSet & TorrentControl::excludedChunksBitSet() const
	{
		if (!cman)
			return BitSet::null;
		else
			return cman->getExcludedBitSet();
	}

	void TorrentControl::saveStats()
	{
		QFile fptr(datadir + "stats");
		if (!fptr.open(IO_WriteOnly))
		{
			Out() << "Warning : can't create stats file" << endl;
			return;
		}

		QTextStream out(&fptr);
		out << "UPLOADED=" << QString::number(up->bytesUploaded()) << ::endl;
		if (stats.running)
		{
			QDateTime now = QDateTime::currentDateTime();
			out << "RUNNING_TIME_DL=" << (running_time_dl + time_started_dl.secsTo(now)) << ::endl;
			out << "RUNNING_TIME_UL=" << (running_time_ul + time_started_ul.secsTo(now)) << ::endl;
		}
		else
		{
			out << "RUNNING_TIME_DL=" << running_time_dl << ::endl;
			out << "RUNNING_TIME_UL=" << running_time_ul << ::endl;
		}
	}

	void TorrentControl::loadStats()
	{
		QFile fptr(datadir + "stats");
		if (!fptr.open(IO_ReadOnly))
			return;

		QTextStream in(&fptr);
		while (!in.atEnd())
		{
			QString line = in.readLine();
			if (line.startsWith("UPLOADED="))
			{
				bool ok = true;
				Uint64 val = line.mid(9).toULongLong(&ok);
				if (ok)
					up->setBytesUploaded(val);
				else
					Out() << "Warning : can't get bytes uploaded out of line : "
					<< line << endl;
				prev_bytes_ul = val;
			}
			else if (line.startsWith("RUNNING_TIME_DL="))
			{
				bool ok = true;
				unsigned long val  = line.mid(16).toULong(&ok);
				if(ok)
					this->running_time_dl = val;
				else
					Out() << "Warning : can't get running time out of line : "
					<< line << endl;
			}
			else if (line.startsWith("RUNNING_TIME_UL="))
			{
				bool ok = true;
				unsigned long val = line.mid(16).toULong(&ok);
				if(ok)
					this->running_time_ul = val;
				else
					Out() << "Warning : can't get running time out of line : "
					<< line << endl;
			}
		}
	}

	bool TorrentControl::readyForPreview(int start_chunk, int end_chunk)
	{
		if ( !tor->isMultimedia() && !tor->isMultiFile()) return false;

		const BitSet & bs = downloadedChunksBitSet();
		for(int i = start_chunk; i<end_chunk; ++i)
		{
			if ( !bs.get(i) ) return false;
		}
		return true;
	}

	Uint32 TorrentControl::getTimeToNextTrackerUpdate() const
	{
		Uint32 elapsed = tracker_update_timer.getElapsedSinceUpdate();
		if (elapsed <= tracker_update_interval)
			return tracker_update_interval - elapsed;
		else
			return 0;
	}

	void TorrentControl::updateStats()
	{
		stats.num_chunks_downloading = down ? down->numActiveDownloads() : 0;
		stats.num_peers = pman ? pman->getNumConnectedPeers() : 0;
		stats.upload_rate = up && stats.running ? up->uploadRate() : 0;
		stats.download_rate = down && stats.running ? down->downloadRate() : 0;
		stats.bytes_left = cman ? cman->bytesLeft() : 0;
		stats.bytes_uploaded = up ? up->bytesUploaded() : 0;
		stats.bytes_downloaded = down ? down->bytesDownloaded() : 0;
		stats.total_chunks = cman ? cman->getNumChunks() : 0;
		stats.num_chunks_downloaded = cman ? cman->getNumChunks() - cman->chunksExcluded() - cman->chunksLeft() : 0;
		stats.num_chunks_excluded = cman ? cman->chunksExcluded() : 0;
		stats.total_bytes_to_download = (tor && cman) ?	tor->getFileLength() - cman->bytesExcluded() : 0;
		stats.session_bytes_downloaded = stats.bytes_downloaded - prev_bytes_dl;
		stats.session_bytes_uploaded = stats.bytes_uploaded - prev_bytes_ul;
		getSeederInfo(stats.seeders_total,stats.seeders_connected_to);
		getLeecherInfo(stats.leechers_total,stats.leechers_connected_to);
	}

	void TorrentControl::getSeederInfo(Uint32 & total,Uint32 & connected_to) const
	{
		total = 0;
		connected_to = 0;
		if (!pman || !tracker)
			return;

		for (Uint32 i = 0;i < pman->getNumConnectedPeers();i++)
		{
			if (pman->getPeer(i)->isSeeder())
				connected_to++;
		}
		total = tracker->getNumSeeders();
		if (total == 0)
			total = connected_to;
	}

	void TorrentControl::getLeecherInfo(Uint32 & total,Uint32 & connected_to) const
	{
		total = 0;
		connected_to = 0;
		if (!pman || !tracker)
			return;

		for (Uint32 i = 0;i < pman->getNumConnectedPeers();i++)
		{
			if (!pman->getPeer(i)->isSeeder())
				connected_to++;
		}
		total = tracker->getNumLeechers();
		if (total == 0)
			total = connected_to;
	}

	Uint32 TorrentControl::getRunningTimeDL() const
	{
		if (!stats.running || stats.completed)
			return running_time_dl;
		else
			return running_time_dl + time_started_dl.secsTo(QDateTime::currentDateTime());
	}

	Uint32 TorrentControl::getRunningTimeUL() const
	{
		if (!stats.running)
			return running_time_ul;
		else
			return running_time_ul + time_started_ul.secsTo(QDateTime::currentDateTime());
	}

	Uint32 TorrentControl::getNumFiles() const
	{
		if (tor && tor->isMultiFile())
			return tor->getNumFiles();
		else
			return 0;
	}
	
	TorrentFileInterface & TorrentControl::getTorrentFile(Uint32 index)
	{
		if (tor)
			return tor->getFile(index);
		else
			return TorrentFile::null;
	}

}
#include "torrentcontrol.moc"
