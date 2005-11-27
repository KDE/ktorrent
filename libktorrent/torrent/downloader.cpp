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
#include <util/file.h>
#include <util/log.h>
#include "downloader.h"
#include "chunkmanager.h"
#include "torrent.h"
#include "peermanager.h"
#include <util/error.h>
#include "chunkdownload.h"
#include <util/sha1hash.h>
#include "peer.h"
#include "piece.h"
#include "peerdownloader.h"
#include <interfaces/monitorinterface.h>
#include "packetwriter.h"
#include "chunkselector.h"
#include "ipblocklist.h"

namespace bt
{
	
	

	Downloader::Downloader(Torrent & tor,PeerManager & pman,ChunkManager & cman) 
	: tor(tor),pman(pman),cman(cman),downloaded(0),tmon(0)
	{
		chunk_selector = new ChunkSelector(cman,*this,pman);
		Uint64 total = tor.getFileLength();
		downloaded = (total - cman.bytesLeft() - cman.bytesExcluded());
	
		current_chunks.setAutoDelete(true);
		connect(&pman,SIGNAL(newPeer(Peer* )),this,SLOT(onNewPeer(Peer* )));
		connect(&pman,SIGNAL(peerKilled(Peer* )),this,SLOT(onPeerKilled(Peer*)));
	}


	Downloader::~Downloader()
	{
		delete chunk_selector;
	}
	
	void Downloader::pieceRecieved(const Piece & p)
	{
		if (cman.chunksLeft() == 0)
			return;
		
		for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();++j)
		{
			if (p.getIndex() != j->first)
				continue;
			
			ChunkDownload* cd = j->second;
			downloaded += p.getLength();
			if (cd->piece(p))
			{
				if (tmon)
					tmon->downloadRemoved(cd);

				if (!finished(cd))
				{
					// if the chunk fails don't count the bytes downloaded
					if (cd->getChunk()->getSize() > downloaded)
						downloaded = 0;
					else
						downloaded -= cd->getChunk()->getSize();
				}
				current_chunks.erase(p.getIndex());
				return;
			}
		}
	}
	
	void Downloader::update()
	{
		if (cman.chunksLeft() == 0)
			return;
		
		/*
			There are 3 modes :
			- warmup : <= 4 chunks downloaded
			- endgame : at the end of download
			- normal : everything else
		*/
		bool warmup = cman.getNumChunks() - cman.chunksLeft() <= 4;
		bool endgame_mode = cman.chunksLeft() <= current_chunks.count();

		if (warmup)
			warmupUpdate();
		else if (endgame_mode)
			endgameUpdate();
		else
			normalUpdate();
		
		// now see if there aren't any timed out pieces
		for (Uint32 i = 0;i < pman.getNumConnectedPeers();i++)
		{
			Peer* p = pman.getPeer(i);
			p->getPeerDownloader()->checkTimeouts();
		}
	}

	void Downloader::warmupUpdate()
	{
		// first try to assign as many peers to the current crop of chunks
		endgameUpdate();
		// then do a normal update to start some more
		normalUpdate();
	}
	
	void Downloader::normalUpdate()
	{
		for (Uint32 i = 0; i < pman.getNumConnectedPeers();++i)
		{
			PeerDownloader* pd = pman.getPeer(i)->getPeerDownloader();
	
			if (!pd->isNull() && !pd->isChoked() && pd->getNumRequests() < pd->getMaximumOutstandingReqs() - 2)
			{
				//if (pd->getNumGrabbed() == 0 || (pd->getNumGrabbed() == 1 && pd->getNumRequests() < 8))
					downloadFrom(pd);
			}
		}
	}
	
	void Downloader::endgameUpdate()
	{
		for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();++j)
		{
			ChunkDownload* cd = j->second;
			PtrMap<Peer*,PeerDownloader>::iterator i;
			for (Uint32 i = 0; i < pman.getNumConnectedPeers();++i)
			{
				PeerDownloader* pd = pman.getPeer(i)->getPeerDownloader();
					
				if (!pd->isNull() && !pd->isChoked() &&
					pd->hasChunk(cd->getChunk()->getIndex()) &&
					pd->getNumRequests() < pd->getMaximumOutstandingReqs() - 2)
				{
					cd->assignPeer(pd,true);
				}
			}
		}
	}
	
	void Downloader::downloadFrom(PeerDownloader* pd)
	{
		// first see if there are ChunkDownload's which need a PeerDownloader
		for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();++j)
		{
			ChunkDownload* cd = j->second;
			bool ok_to_down = pd->hasChunk(cd->getChunk()->getIndex());
			if (!ok_to_down)
				continue;

			// if cd hasn't got a downloader or when the current
			// downloader has snubbed him
			// assign him pd
			const Peer* p = cd->getCurrentPeer();
			if (cd->getNumDownloaders() == 0 || (p && p->isSnubbed()))
			{
				cd->assignPeer(pd,false);
				return;
			}
		}
		
	//	if (current_chunks.count() > 2*pdowners.count())
	//		return;

		Uint32 chunk = 0;
		if (chunk_selector->select(pd,chunk))
		{
			ChunkDownload* cd = new ChunkDownload(cman.getChunk(chunk));
			current_chunks.insert(chunk,cd);
			cd->assignPeer(pd,false);
			if (tmon)
				tmon->downloadStarted(cd);
		}
	}

	bool Downloader::areWeDownloading(Uint32 chunk) const
	{
		return current_chunks.find(chunk) != 0;
	}
	
	void Downloader::onNewPeer(Peer* peer)
	{		
		PeerDownloader* pd = peer->getPeerDownloader();
		connect(pd,SIGNAL(downloaded(const Piece& )),
				this,SLOT(pieceRecieved(const Piece& )));
	}

	void Downloader::onPeerKilled(Peer* peer)
	{
		PeerDownloader* pd = peer->getPeerDownloader();
		if (pd)
		{
			for (CurChunkItr i = current_chunks.begin();i != current_chunks.end();++i)
			{
				ChunkDownload* cd = i->second;
				cd->peerKilled(pd);
			}
		}
	}
	
	bool Downloader::finished(ChunkDownload* cd)
	{
		Chunk* c = cd->getChunk();
		// verify the data
		SHA1Hash h = SHA1Hash::generate(c->getData(),c->getSize());
		if (tor.verifyHash(h,c->getIndex()))
		{
			// hash ok so save it
			try
			{
				cman.saveChunk(c->getIndex());
				Out() << "Chunk " << c->getIndex() << " downloaded " << endl;
				// tell everybody we have the Chunk
				for (Uint32 i = 0;i < pman.getNumConnectedPeers();i++)
				{
					pman.getPeer(i)->getPacketWriter().sendHave(c->getIndex());
				}
			}
			catch (Error & e)
			{
				Out() << "Error " << e.toString() << endl;
				emit ioError(e.toString());
				return false;
			}
		}
		else
		{
			Out() << "Hash verification error on chunk "  << c->getIndex() << endl;
			Out() << "Is        : " << h << endl;
			Out() << "Should be : " << tor.getHash(c->getIndex()) << endl;
			Uint32 pid;
			if (cd->getOnlyDownloader(pid))
			{
				Peer* p = pman.findPeer(pid);
				if (!p)
					return false;
				QString IP(p->getIPAddresss());
				Out() << "Peer " << IP << " sent bad data" << endl;
				IPBlocklist & ipfilter = IPBlocklist::instance();
				ipfilter.insert( IP );
				if (ipfilter.isBlocked( IP ))
				{
					Out() << "Peer " << IP << " has been blacklisted" << endl;
					p->kill(); 
				}
			}
			return false;
		}
		return true;
	}
	
	void Downloader::clearDownloads()
	{
		current_chunks.clear();
	}
	
	Uint32 Downloader::downloadRate() const
	{
		// sum of the download rate of each peer
		Uint32 rate = 0;
		for (Uint32 i = 0;i < pman.getNumConnectedPeers();i++)
		{
			Peer* p = pman.getPeer(i);
			rate += p->getDownloadRate();
		}
		return rate;
	}
	
	void Downloader::setMonitor(kt::MonitorInterface* tmo)
	{
		tmon = tmo;
		if (!tmon)
			return;

		for (CurChunkItr i = current_chunks.begin();i != current_chunks.end();++i)
		{
			ChunkDownload* cd = i->second;
			tmon->downloadStarted(cd);
		}
	}

	void Downloader::saveDownloads(const QString & file)
	{
		File fptr;
		if (!fptr.open(file,"wb"))
			return;

		// Save all the current downloads to a file
		Uint32 num = current_chunks.count();
		fptr.write(&num,sizeof(Uint32));

		Out() << "Saving " << current_chunks.count() << " chunk downloads" << endl;
		for (CurChunkItr i = current_chunks.begin();i != current_chunks.end();++i)
		{
			Uint32 ch = i->first;
			fptr.write(&ch,sizeof(Uint32));
			ChunkDownload* cd = i->second;
			cd->save(fptr);
		}
	}

	void Downloader::loadDownloads(const QString & file)
	{
		// don't load stuff if download is finished
		if (cman.chunksLeft() == 0)
			return;
		
		// Load all partial downloads
		File fptr;
		if (!fptr.open(file,"rb"))
			return;

		// recalculate downloaded bytes
		downloaded = (tor.getFileLength() - cman.bytesLeft() - cman.bytesExcluded());

		Uint32 num = 0;
		fptr.read(&num,sizeof(Uint32));

		Out() << "Loading " << num  << " active chunk downloads" << endl;
		for (Uint32 i = 0;i < num;i++)
		{
			Uint32 ch = 0;
			fptr.read(&ch,sizeof(Uint32));
			Out() << "Loading chunk " << ch << endl;
	
			if (!cman.getChunk(ch) || current_chunks.contains(ch))
			{
				Out() << "Illegal chunk " << ch << endl;
				return;
			}
			ChunkDownload* cd = new ChunkDownload(cman.getChunk(ch));
			current_chunks.insert(ch,cd);
			cd->load(fptr);
			downloaded += cd->bytesDownloaded();
			if (tmon)
				tmon->downloadStarted(cd);
		}
	}

	bool Downloader::isFinished() const
	{
		return cman.chunksLeft() == 0;
	}

	void Downloader::onExcluded(Uint32 from,Uint32 to)
	{
		for (Uint32 i = from;i <= to;i++)
		{
			ChunkDownload* cd = current_chunks.find(i);
			if (!cd)
				continue;
			cd->cancelAll();
			if (tmon)
				tmon->downloadRemoved(cd);
			current_chunks.erase(i);
		}
	}
}
#include "downloader.moc"
