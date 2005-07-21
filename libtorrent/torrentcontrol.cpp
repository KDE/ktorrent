/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <qfile.h>
#include <klocale.h>
#include <libutil/fileops.h>
#include "downloader.h"
#include "uploader.h"
#include "tracker.h"
#include "chunkmanager.h"
#include "torrent.h"
#include "bdecoder.h"
#include "bnode.h"
#include "peermanager.h"
#include <libutil/error.h>
#include <libutil/log.h>
#include <libutil/functions.h>
#include "torrentcontrol.h"
#include "bitset.h"
#include "peer.h"
#include "choker.h"
#include "torrentmonitor.h"

#include "globals.h"
#include "packetwriter.h"
#include "httptracker.h"
#include "udptracker.h"
#include "downloadcap.h"




namespace bt
{
	
	Uint16 TorrentControl::initial_port = MIN_PORT;
	
	TorrentControl::TorrentControl() 
	: tor(0),tracker(0),cman(0),pman(0),down(0),up(0),choke(0),tmon(0)
	{
		running = false;
		started = false;
		saved = false;
		num_tracker_attempts = 0;
		old_datadir = QString::null;
		tracker_update_interval = 120000;
	}
	
	


	TorrentControl::~TorrentControl()
	{
		if (isRunning())
			stop();
		
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
		pman->update();
		bool comp = completed;
		
		up->update();
		if (!completed)
			down->update();
		
		completed = cman->chunksLeft() == 0;
		if (completed && !comp)
		{
			updateTracker("completed");
			finished(this);
			down->clearDownloaders();
		}
		updateStatusMsg();
		
		cman->checkMemoryUsage();
		pman->clearDeadPeers();
		
		if (tracker_update_timer.getElapsedSinceUpdate() >= tracker_update_interval)
		{
			updateTracker();
			tracker_update_timer.update();
		}
		
		if (choker_update_timer.getElapsedSinceUpdate() >= 10000)
		{
			doChoking();
			choker_update_timer.update();
		}

		DownloadCap::instance().update();
	}
	
	
	void TorrentControl::setMonitor(TorrentMonitor* tmo)
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
		completed = false;
		running = false;
		if (!datadir.endsWith(DirSeparator()))
			datadir += DirSeparator();
		
		
		tor = new Torrent();
		tor->load(torrent);
		if (!bt::Exists(datadir))
			bt::MakeDir(datadir);
	
		//QString cache_file = datadir + "cache";
		QString tor_copy = datadir + "torrent";
		
		if (tor_copy != torrent)
			bt::CopyFile(torrent,tor_copy);
		
		do
		{
			if (pman)
			{
				delete pman;
				pman = 0;
			}
			port = initial_port;
			initial_port++;
			pman = new PeerManager(*tor,port);
		}while (!pman->ok());

		if (tor->getTrackerURL(true).protocol() == "udp")
			tracker = new UDPTracker(this);
		else
			tracker = new HTTPTracker(this);
		
		cman = new ChunkManager(*tor,datadir);
		if (bt::Exists(datadir + "index"))
			cman->loadIndexFile();
		else
			cman->createFiles();
		
		completed = cman->chunksLeft() == 0;
		
		
		down = new Downloader(*tor,*pman,*cman);
		up = new Uploader(*cman);
		choke = new Choker(*pman);
	
		
		connect(pman,SIGNAL(newPeer(Peer* )),this,SLOT(onNewPeer(Peer* )));
		connect(pman,SIGNAL(peerKilled(Peer* )),this,SLOT(onPeerRemoved(Peer* )));
		saved = cman->hasBeenSaved();
		updateStatusMsg();
	}

	void TorrentControl::setTrackerTimerInterval(Uint32 interval)
	{
		tracker_update_interval = interval;
	}

	void TorrentControl::trackerResponse(const QByteArray & data)
	{
		BNode* n = 0;
		try
		{
			Out() << "Tracker updated" << endl;
		//	Out() << "Reply size = " << data.size() << endl;
		//	Out() << "Data = " << QString(data) << endl;
			BDecoder dec(data);
			n = dec.decode();
			
			if (!n || n->getType() != BNode::DICT)
				throw Error("Parse Error");
			
			BDictNode* dict = (BDictNode*)n;
			if (dict->getData("failure reason"))
			{
				BValueNode* vn = dict->getValue("failure reason");
				if (pman->getNumConnectedPeers() == 0)
					trackerError(this,i18n("The tracker sent back the following error: %1").arg(vn->data().toString()));
				else
					trackerResponseError();
				return;
			}
			
			BValueNode* vn = dict->getValue("interval");
			
			if (!vn)
				throw Error("Parse Error");
			
			Uint32 update_time = vn->data().toInt() > 300 ? 300 : vn->data().toInt();
			
			Out() << "Next update in " << update_time << " seconds" << endl;

			setTrackerTimerInterval(update_time * 1000);

			pman->trackerUpdate(dict);
			delete n;
			num_tracker_attempts = 0;
			updateStatusMsg();
		}
		catch (Error & e)
		{
			Out() << "Error : " << e.toString() << endl;
			if (n)
				n->printDebugInfo();
			
		/*	Out() << "Data : " << endl;
			Out() << QString(data) << endl;*/
			delete n;

			if (num_tracker_attempts >= tor->getNumTrackerURLs() &&
						 trackerevent != "stopped")
			{
				if (pman->getNumConnectedPeers() == 0)
				{
					trackerError(this,i18n("The tracker %1 did not send a proper response"
						", stopping download").arg(last_tracker_url.prettyURL()));
				}
				else
				{
					updateStatusMsg();
				}
			}
			else if (trackerevent != "stopped")
			{
				updateTracker(trackerevent,false);
			}
		}
	}

	void TorrentControl::trackerResponse(Uint32 interval,Uint32 leechers,Uint32 seeders,Uint8* ppeers)
	{
		Out() << "Tracker updated" << endl;
		setTrackerTimerInterval(interval * 1000);
		pman->trackerUpdate(seeders,leechers,ppeers);
		updateStatusMsg();
	}

	
	void TorrentControl::trackerResponseError()
	{
		Out() << "Tracker Response Error" << endl;
		if (num_tracker_attempts >= tor->getNumTrackerURLs() &&
		    trackerevent != "stopped")
		{
			if (pman->getNumConnectedPeers() == 0)
			{
				trackerError(this,i18n("The tracker %1 is down, stopping download.")
					.arg(last_tracker_url.prettyURL()));
			}
			else
			{
				updateStatusMsg();
			}

						}
		else if (trackerevent != "stopped")
		{
			updateTracker(trackerevent,false);
		}
	}
	
	void TorrentControl::updateTracker(const QString & ev,bool last_succes)
	{
		updateStatusMsg(i18n("Doing tracker request"));
		trackerevent = ev;
		if (!tor || !tracker || !down || !up)
			return;
		
		KURL url = tor->getTrackerURL(last_succes);
		last_tracker_url = url;
		tracker->setData(tor->getInfoHash(),tor->getPeerID(),port,
						 up->bytesUploaded(),down->bytesDownloaded(),
						 cman->bytesLeft(),ev);

		tracker->doRequest(url);
		num_tracker_attempts++;
	}
	

	void TorrentControl::onNewPeer(Peer* p)
	{
		connect(p,SIGNAL(request(const Request& )),up,SLOT(addRequest(const Request& )));
		connect(p,SIGNAL(canceled(const Request& )),up,SLOT(cancel(const Request& )));
		BitSet bs;
		cman->toBitSet(bs);
		p->getPacketWriter().sendBitSet(bs);
		up->addPeer(p);
		if (tmon)
			tmon->peerAdded(p);
	}
	
	void TorrentControl::onPeerRemoved(Peer* p)
	{
		up->removePeer(p);
		if (tmon)
			tmon->peerRemoved(p);
	}

	void TorrentControl::doChoking()
	{
		choke->update(cman->bytesLeft() == 0);
	}
	
	void TorrentControl::start()
	{
		num_tracker_attempts = 0;
		updateTracker("started");
		pman->start();
		down->loadDownloads(datadir + "current_chunks");
		running = true;
		started = true;
		tracker_update_timer.update();
		choker_update_timer.update();
	}
	
	void TorrentControl::stop()
	{
		if (running)
		{
			if (num_tracker_attempts < tor->getNumTrackerURLs())
				updateTracker("stopped");
		
			if (tmon)
				tmon->stopped();
			
			down->saveDownloads(datadir + "current_chunks");
			down->clearDownloads();
		}
		up->removeAllPeers();

		pman->stop();
		pman->closeAllConnections();
		pman->clearDeadPeers();
		running = false;
		updateStatusMsg();
	}
	
	QString TorrentControl::getTorrentName() const
	{
		if (tor)
			return tor->getNameSuggestion();
		else
			return QString::null;
	}
	
	Uint32 TorrentControl::getBytesDownloaded() const
	{
		if (down)
			return down->bytesDownloaded();
		else
			return 0;
	}
	
	Uint32 TorrentControl::getBytesUploaded() const
	{
		if (up)
			return up->bytesUploaded();
		else
			return 0;
	}
	
	Uint32 TorrentControl::getBytesLeft() const
	{
		if (cman)
			return cman->bytesLeft();
		else
			return 0;
	}
	
	Uint32 TorrentControl::getDownloadRate() const
	{
		if (down && running)
			return down->downloadRate();
		else
			return 0;
	}
	
	Uint32 TorrentControl::getUploadRate() const
	{
		if (up && running)
			return up->uploadRate();
		else
			return 0;
	}
	
	Uint32 TorrentControl::getNumPeers() const
	{
		if (pman)
			return pman->getNumConnectedPeers();
		else
			return 0;
	}
	
	Uint32 TorrentControl::getNumChunksDownloading() const
	{
		if (down)
			return down->numActiveDownloads();
		else
			return 0;
	}
	
	void TorrentControl::reconstruct(const QString & dir)
	{
		if (saved)
			return;
	
		cman->save(dir);
		saved = true;
	}
	
	bool TorrentControl::isMultiFileTorrent() const
	{
		return tor->isMultiFile();
	}
	
	void TorrentControl::setInitialPort(Uint16 port)
	{
		initial_port = port;
	}
	
	Uint32 TorrentControl::getTotalChunks() const
	{
		return cman->getNumChunks();
	}
	
	Uint32 TorrentControl::getNumChunksDownloaded() const
	{
		return cman->getNumChunks() - cman->chunksLeft();
	}

	Uint32 TorrentControl::getTotalBytes() const
	{
		return tor->getFileLength();
	}

	QString TorrentControl::getStatus() const
	{
		return status_msg;
	}
	
	void TorrentControl::setStatus(const QString & s)
	{
		status_msg = s;
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

		// tell the chunkmanager that the datadir has changed
		cman->changeDataDir(nd);
		
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

	void TorrentControl::updateStatusMsg(const QString & def)
	{
		if (!started)
			setStatus(i18n("Not started"));
		else if (!running && completed)
			setStatus(i18n("Download complete"));
		else if (!running)
			setStatus(i18n("Download stopped"));
		else if (running && completed)
		{
			if (def.length() > 0)
				setStatus(i18n("Seeding + %1").arg(def));
			else
				setStatus(i18n("Seeding"));
		}
		else if (running)
		{
			QString m = down->downloadRate() > 0 ? i18n("Downloading") : i18n("Stalled");
			if (def.length() > 0)
				setStatus(i18n("%1 + %2").arg(m).arg(def));
			else
				setStatus(m);
		}
	}

	void TorrentControl::toBitSet(BitSet & bs)
	{
		if (cman)
			cman->toBitSet(bs);
	}
}
#include "torrentcontrol.moc"
