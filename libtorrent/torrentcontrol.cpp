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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <qfile.h>
#include <kio/netaccess.h>
#include <kprogress.h>
#include "downloader.h"
#include "uploader.h"
#include "tracker.h"
#include "chunkmanager.h"
#include "torrent.h"
#include "bdecoder.h"
#include "bnode.h"
#include "peermanager.h"
#include "error.h"
#include "torrentcontrol.h"
#include "bitset.h"
#include "peer.h"
#include "choker.h"
#include "filereconstructor.h"
#include "torrentmonitor.h"
#include "log.h"
#include "globals.h"
#include "packetwriter.h"


namespace bt
{
	
	Uint16 TorrentControl::initial_port = MIN_PORT;
	
	TorrentControl::TorrentControl() 
	: tor(0),tracker(0),cman(0),pman(0),down(0),up(0),choke(0),tmon(0)
	{
		running = false;
		started = false;
		saved = false;
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
	
	void TorrentControl::setMonitor(TorrentMonitor* tmo)
	{
		tmon = tmo;
		down->setMonitor(tmon);
	}
	
	void TorrentControl::init(const QString & torrent,const QString & ddir)
	{
		datadir = ddir;
		completed = false;
		running = false;
		if (!datadir.endsWith("/"))
			datadir += "/";
		
		
		tor = new Torrent();
		tor->load(torrent);
	
		QString cache_file = datadir + "cache";
		QString tor_copy = datadir + "torrent";
		
		if (tor_copy != torrent)
			KIO::NetAccess::file_copy(torrent,tor_copy);
		
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
		
		tracker = new Tracker();
		cman = new ChunkManager(*tor,datadir);
		if (KIO::NetAccess::exists(datadir + "index",true,0))
			cman->loadIndexFile();
		else
			cman->createFiles();
		
		completed = cman->chunksLeft() == 0;
		
		
		down = new Downloader(*tor,*pman,*cman);
		up = new Uploader(*cman);
		choke = new Choker(*pman);
	
		connect(tracker,SIGNAL(requestError()),
				this,SLOT(trackerResponseError()));
		connect(tracker,SIGNAL(response(const QByteArray& )),
				this,SLOT(trackerResponse(const QByteArray& )));
		
		connect(&tracker_update_timer,SIGNAL(timeout()),this,SLOT(updateTracker()));
		connect(&choker_update_timer,SIGNAL(timeout()),this,SLOT(doChoking()));
		connect(&update_timer,SIGNAL(timeout()),this,SLOT(update()));
		connect(pman,SIGNAL(newPeer(Peer* )),this,SLOT(onNewPeer(Peer* )));
		connect(pman,SIGNAL(peerKilled(Peer* )),this,SLOT(onPeerRemoved(Peer* )));
	}

	void TorrentControl::trackerResponse(const QByteArray & data)
	{
		BNode* n = 0;
		try
		{
			Out() << "Tracker updated" << endl;
			BDecoder dec(data);
			n = dec.decode();
			
			if (!n || n->getType() != BNode::DICT)
				throw Error("Parse Error");
			
			BDictNode* dict = (BDictNode*)n;
			if (dict->getData("failure reason"))
			{
				trackerResponseError();
				return;
			}
			
			BNode* tmp = dict->getData("interval");
			
			if (!tmp || tmp->getType() != BNode::VALUE)
				throw Error("Parse Error");
			
			BValueNode* vn = (BValueNode*)tmp;
			Uint32 update_time = vn->data().toInt() > 300 ? 300 : vn->data().toInt();
			
			Out() << "Next update in " << update_time << " seconds" << endl;
			tracker_update_timer.changeInterval(update_time * 1000);
			
			pman->trackerUpdate(dict);
			delete n;
		}
		catch (Error & e)
		{
			Out() << "Error : " << e.toString() << endl;
			if (n)
				n->printDebugInfo();
			
		/*	Out() << "Data : " << endl;
			Out() << QString(data) << endl;*/
			delete n;
		}
	}
	
	void TorrentControl::trackerResponseError()
	{
		Out() << "Tracker Response Error" << endl;
		updateTracker(trackerevent,false);
	}
	
	void TorrentControl::updateTracker(const QString & ev,bool last_succes)
	{
		trackerevent = ev;
		if (!tor || !tracker || !down || !up)
			return;
		
		KURL url = tor->getTrackerURL(last_succes);

		const SHA1Hash & info_hash = tor->getInfoHash();
		QString query = QString("&info_hash=") + info_hash.toURLString();
		url.addQueryItem("peer_id",tor->getPeerID().toString());
		url.addQueryItem("port",QString::number(port));
		url.addQueryItem("uploaded",QString::number(up->bytesUploaded()));
		url.addQueryItem("downloaded",QString::number(down->bytesDownloaded()));
		url.addQueryItem("left",QString::number(cman->bytesLeft()));
		//url.addQueryItem("compact","1");
		//url.addQueryItem("numwant","100");
		if (ev != QString::null)
			url.addQueryItem("event",ev);
		
	//	Out() << "Tracker Update " << url << query << endl;
	//	Out() << info_hash.toURLString() << endl;
	//	Out() << query << endl;
		Uint16 http_port = url.port();
		if (http_port == 0)
			http_port = 80;
		tracker->doRequest(url.host(),url.encodedPathAndQuery() + query,http_port);
	}
	
	void TorrentControl::update()
	{
		pman->connectToPeers();
		
		bool comp = completed;
		pman->clearDeadPeers();
		up->update();
		if (!completed)
			down->update();
		
		pman->updateSpeed();
		completed = cman->chunksLeft() == 0;
		if (completed && !comp)
		{
			updateTracker("completed");
			finished(this);
		}
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
		updateTracker("started");
		tracker_update_timer.start(120000);
		choker_update_timer.start(10000);
		update_timer.start(100);
		pman->start();
		down->loadDownloads(datadir + "current_chunks");
		running = true;
		started = true;
	}
	
	void TorrentControl::stop()
	{
		updateTracker("stopped");
		if (tmon)
			tmon->stopped();

		down->saveDownloads(datadir + "current_chunks");
		down->clearDownloads();
		up->removeAllPeers();
		tracker_update_timer.stop();
		choker_update_timer.stop();
		update_timer.stop();
		pman->stop();
		pman->closeAllConnections();
		pman->clearDeadPeers();
		running = false;
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
	
	void TorrentControl::reconstruct(const QString & file,KProgressDialog* dlg)
	{
		FileReconstructor fr(*tor,*cman);
		if (dlg)
		{
			KProgress* pb = dlg->progressBar();
			pb->setTotalSteps(cman->getNumChunks()-1);
			connect(&fr,SIGNAL(completed(int )),pb,SLOT(setProgress(int )));
		}
		fr.reconstruct(file);
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
}
#include "torrentcontrol.moc"
