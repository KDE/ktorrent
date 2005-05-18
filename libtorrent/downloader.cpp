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
#include "file.h"
#include "log.h"
#include "downloader.h"
#include "chunkmanager.h"
#include "torrent.h"
#include "peermanager.h"
#include "error.h"
#include "chunkdownload.h"
#include "sha1hash.h"
#include "peer.h"
#include "piece.h"
#include "peerdownloader.h"
#include "torrentmonitor.h"
#include "packetwriter.h"

namespace bt
{
	
	

	Downloader::Downloader(Torrent & tor,PeerManager & pman,ChunkManager & cman) 
	: tor(tor),pman(pman),cman(cman),downloaded(0),tmon(0)
	{
		Uint32 total = tor.getFileLength();
		downloaded = total - cman.bytesLeft();
		endgame_mode = false;
		pdowners.setAutoDelete(true);
		current_chunks.setAutoDelete(true);
		connect(&pman,SIGNAL(newPeer(Peer* )),this,SLOT(onNewPeer(Peer* )));
		connect(&pman,SIGNAL(peerKilled(Peer* )),this,SLOT(onPeerKilled(Peer*)));
	}


	Downloader::~Downloader()
	{
	}
	
	void Downloader::pieceRecieved(const Piece & p)
	{
		for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();j++)
		{
			if (p.getIndex() != j->first)
				continue;
			
			ChunkDownload* cd = j->second;
			downloaded += p.getLength();
			if (cd->piece(p))
			{
				finished(cd);
				if (tmon)
					tmon->downloadRemoved(cd);
				current_chunks.erase(p.getIndex());
				return;
			}
		}
	}
	
	void Downloader::update()
	{
		if (cman.bytesLeft() == 0)
			return;
		
		endgame_mode = 
				cman.chunksLeft() <= current_chunks.count() &&
				cman.chunksLeft() < 20;

		if (endgame_mode)
			endgameUpdate();
		else
			normalUpdate();
	}
	
	void Downloader::normalUpdate()
	{
		PtrMap<Peer*,PeerDownloader>::iterator i;
		for (i = pdowners.begin(); i != pdowners.end();i++)
		{
			PeerDownloader* pd = i->second;
			if (!pd->isNull() && !pd->isChoked())
			{
				if (pd->getNumGrabbed() == 0 || (pd->getNumGrabbed() == 1 && pd->getNumRequests() < 4))
					downloadFrom(pd);
			}
		}
	}
	
	void Downloader::endgameUpdate()
	{
		for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();j++)
		{			
			ChunkDownload* cd = j->second;
			PtrMap<Peer*,PeerDownloader>::iterator i;
			for (i = pdowners.begin(); i != pdowners.end();i++)
			{
				PeerDownloader* pd = i->second;
					
				if (!pd->isNull() && !pd->isChoked() &&
					pd->hasChunk(cd->getChunk()->getIndex()) &&
					pd->getNumRequests() < 8)
				{
					cd->assignPeer(pd,true);
				}
			}
		}
	}
	
	void Downloader::downloadFrom(PeerDownloader* pd)
	{
		for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();j++)
		{
			ChunkDownload* cd = j->second;
			bool ok_to_down = pd->hasChunk(cd->getChunk()->getIndex());
			if (!ok_to_down)
				continue;
			
			const Peer* p = cd->getCurrentPeer();
			if (cd->getNumDownloaders() == 0 || (p && p->isSnubbed()))
			{
				pd->grab();
				cd->assignPeer(pd,false);
				return;
			}
		}
		
		if (current_chunks.count() > 2*pdowners.count())
			return;
	
		Uint32 s = rand() % tor.getNumChunks();
		Uint32 i = s;
		BitSet bs;
		cman.toBitSet(bs);
		do
		{
			
			if (pd->hasChunk(i) && !current_chunks.find(i) && !bs.get(i))
			{
				ChunkDownload* cd = new ChunkDownload(cman.getChunk(i));
				current_chunks.insert(i,cd);
				pd->grab();
				cd->assignPeer(pd,false);
				if (tmon)
					tmon->downloadStarted(cd);
				return;
			}
			i = (i + 1) % tor.getNumChunks();
		}while (s != i);
	}
	
	void Downloader::onNewPeer(Peer* peer)
	{
		PeerDownloader* pd = new PeerDownloader(peer);
		connect(pd,SIGNAL(downloaded(const Piece& )),
				this,SLOT(pieceRecieved(const Piece& )));
		pdowners.insert(peer,pd);
	}

	void Downloader::onPeerKilled(Peer* peer)
	{
		PeerDownloader* pd = pdowners.find(peer);
		if (pd)
		{
			for (CurChunkItr i = current_chunks.begin();i != current_chunks.end();i++)
			{
				ChunkDownload* cd = i->second;
				cd->peerKilled(pd);
			}
			pdowners.erase(peer);
		}
	}
	
	void Downloader::finished(ChunkDownload* cd)
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
				for (Uint32 i = 0;i < pman.getNumConnectedPeers();i++)
				{
					pman.getPeer(i)->getPacketWriter().sendHave(c->getIndex());
				}
			}
			catch (Error & e)
			{
				Out() << "Error " << e.toString() << endl;
			}
		}
		else
		{
			Out() << "Hash verification error on chunk "  << c->getIndex() << endl;
			Out() << "Is        : " << h << endl;
			Out() << "Should be : " << tor.getHash(c->getIndex()) << endl;
		}
		
	}
	
	void Downloader::clearDownloads()
	{
		current_chunks.clear();
	}
	
	Uint32 Downloader::downloadRate() const
	{
		Uint32 rate = 0;
		for (Uint32 i = 0;i < pman.getNumConnectedPeers();i++)
		{
			Peer* p = pman.getPeer(i);
			rate += p->getDownloadRate();
		}
		return rate;
	}
	
	void Downloader::setMonitor(TorrentMonitor* tmo)
	{
		tmon = tmo;
		if (!tmon)
			return;

		for (CurChunkItr i = current_chunks.begin();i != current_chunks.end();i++)
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


		Uint32 num = current_chunks.count();
		fptr.write(&num,sizeof(Uint32));

		Out() << "Saving " << current_chunks.count() << " chunk downloads" << endl;
		for (CurChunkItr i = current_chunks.begin();i != current_chunks.end();i++)
		{
			Uint32 ch = i->first;
			fptr.write(&ch,sizeof(Uint32));
			ChunkDownload* cd = i->second;
			cd->save(fptr);
		}
	}

	void Downloader::loadDownloads(const QString & file)
	{
		File fptr;
		if (!fptr.open(file,"rb"))
			return;


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
			if (tmon)
				tmon->downloadStarted(cd);
		}
	}
}
;
#include "downloader.moc"
