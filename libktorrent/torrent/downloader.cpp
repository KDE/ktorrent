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
#include <util/array.h>
#include "peer.h"
#include "piece.h"
#include "peerdownloader.h"
#include <interfaces/monitorinterface.h>
#include "packetwriter.h"
#include "chunkselector.h"
#include "ipblocklist.h"
#include "ktversion.h"

namespace bt
{
	
	

	Downloader::Downloader(Torrent & tor,PeerManager & pman,ChunkManager & cman) 
	: tor(tor),pman(pman),cman(cman),downloaded(0),tmon(0)
	{
		chunk_selector = new ChunkSelector(cman,*this,pman);
		Uint64 total = tor.getFileLength();
		downloaded = (total - cman.bytesLeft() - cman.bytesExcluded());
		curr_chunks_dowloaded = 0;
	
		current_chunks.setAutoDelete(true);
		connect(&pman,SIGNAL(newPeer(Peer* )),this,SLOT(onNewPeer(Peer* )));
		connect(&pman,SIGNAL(peerKilled(Peer* )),this,SLOT(onPeerKilled(Peer*)));
		num_non_idle = 0;
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
			// if the chunk is not in memory, reload it
			if (cd->getChunk()->getStatus() == Chunk::ON_DISK)
			{
				cman.prepareChunk(cd->getChunk(),true);
			}
			
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
			else
			{
				// save to disk again, if it is idle
				if (cd->isIdle() && cd->getChunk()->getStatus() == Chunk::MMAPPED)
				{
					cman.saveChunk(cd->getChunk()->getIndex(),false);
				}
			}
		}
	}
	
	void Downloader::update()
	{
		if (cman.chunksLeft() == 0)
			return;
		
		/*
			Normal update should now handle all modes properly.
		*/
		normalUpdate();
		
		// now see if there aren't any timed out pieces
		for (Uint32 i = 0;i < pman.getNumConnectedPeers();i++)
		{
			Peer* p = pman.getPeer(i);
			p->getPeerDownloader()->checkTimeouts();
		}
	}

	
	void Downloader::normalUpdate()
	{
		for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();++j)
		{
			ChunkDownload* cd = j->second;
			if (cd->isIdle()) // idle chunks do not need to be in memory
			{
				Chunk* c = cd->getChunk();
				if (c->getStatus() == Chunk::MMAPPED)
				{
					cman.saveChunk(cd->getChunk()->getIndex(),false);
					if (num_non_idle > 0) num_non_idle--;
				}
			} 
			else if (cd->isChoked())
			{
				cd->releaseAllPDs();
				Chunk* c = cd->getChunk();
				if (c->getStatus() == Chunk::MMAPPED)
				{
					cman.saveChunk(cd->getChunk()->getIndex(),false);
					if (num_non_idle > 0) num_non_idle--;
				}
			}
		}
		
		for (Uint32 i = 0; i < pman.getNumConnectedPeers();++i)
		{
			PeerDownloader* pd = pman.getPeer(i)->getPeerDownloader();
			
			if (pd->isNull() || pd->isChoked())
				continue;
	
			bool ok = pd->getNumGrabbed() < pd->getMaxChunkDownloads() && 
					pd->getNumRequests() < pd->getMaximumOutstandingReqs();
			if (ok)
			{
				downloadFrom(pd);
			}
		}
		
		for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();++j)
		{
			ChunkDownload* cd = j->second;
			if (cd->isChoked())
				cd->releaseAllPDs();
		}
	}

	void Downloader::downloadFrom(PeerDownloader* pd)
	{
		// calculate the max memory usage
		Uint32 max = 1024 * 1024;
		if (cman.getNumChunks() - cman.chunksLeft() <= 4)
		{
			// in the beginning stick to four chunks for warmup mode
			max = tor.getChunkSize() * 4;
		}
		else
		{
			switch (mem_usage)
			{	
				case 1: // Medium
					max *= 20; // 20 MB
					break;
				case 2: // High
					max *= 40; // 40 MB
					break;
				case 0: // LOW
				default:
					max *= 10; // 10 MB
					break;
			}
		}
		
		ChunkDownload* sel = 0;
		Uint32 sel_left = 0xFFFFFFFF;
		// first see if there are ChunkDownload's which need a PeerDownloader
		for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();++j)
		{
			ChunkDownload* cd = j->second;
			if (!pd->hasChunk(cd->getChunk()->getIndex()))
				continue;

			// if cd hasn't got a downloader we can try it
			if (cd->getNumDownloaders() == 0 && num_non_idle * tor.getChunkSize() <= max)
			{
				// lets favor the ones which are nearly finished
				if (!sel || cd->getTotalPieces() - cd->getPiecesDownloaded() < sel_left)
				{
					sel = cd;
					sel_left = sel->getTotalPieces() - sel->getPiecesDownloaded();
				}
			}
		}
		
		if (sel)
		{
			// if it is on disk, reload it
			if (sel->getChunk()->getStatus() == Chunk::ON_DISK)
			{
				cman.prepareChunk(sel->getChunk(),true);
				num_non_idle++;
			}
				
			sel->assignPeer(pd);
			return;
		}
		
		bool limit_exceeded = num_non_idle * tor.getChunkSize() >= max;
		Uint32 chunk = 0;
		if (!limit_exceeded && chunk_selector->select(pd,chunk))
		{
			Chunk* c = cman.getChunk(chunk);
			if (cman.prepareChunk(c))
			{
				ChunkDownload* cd = new ChunkDownload(c);
				current_chunks.insert(chunk,cd);
				cd->assignPeer(pd);
				num_non_idle++;
				if (tmon)
					tmon->downloadStarted(cd);
			}
		}
		else if (pd->getNumGrabbed() == 0)
		{ 
			// If the peer hasn't got a chunk we want, 
			// try to assign it to a chunk we are currently downloading 
			// but we only do this if it hasn't been assigned to anything
			ChunkDownload *cdmin=NULL; 
			for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();++j) 
			{ 
				ChunkDownload* cd = j->second; 
				if (pd->hasChunk(cd->getChunk()->getIndex())) 
				{ 
					if (cd->containsPeer(pd)) 
						continue; 
					if (cdmin==NULL) 
						cdmin=cd; 
					else if (cd->getNumDownloaders()<cdmin->getNumDownloaders()) 
						cdmin=cd; 
 
				} 
			} 
			if (cdmin) 
			{
				// if it is on disk, reload it
				if (cdmin->getChunk()->getStatus() == Chunk::ON_DISK)
				{
					cman.prepareChunk(cdmin->getChunk(),true);
				}
				
				if (cdmin->isIdle())
					num_non_idle++;
				
				cdmin->assignPeer(pd); 
			}
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
				if (num_non_idle > 0) num_non_idle--;
				cman.saveChunk(c->getIndex());
				Out() << "Chunk " << c->getIndex() << " downloaded " << endl;
				// tell everybody we have the Chunk
				for (Uint32 i = 0;i < pman.getNumConnectedPeers();i++)
				{
				//	if (!pman.getPeer(i)->isSeeder())
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
		for (CurChunkItr i = current_chunks.begin();i != current_chunks.end();++i)
		{
			Uint32 ch = i->first;
			Chunk* c = i->second->getChunk();
			if (c->getStatus() == Chunk::MMAPPED)
				cman.saveChunk(ch,false);
			
			c->setStatus(Chunk::NOT_DOWNLOADED);
		}
		current_chunks.clear();
		num_non_idle = 0;
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
		CurrentChunksHeader hdr;
		hdr.magic = CURRENT_CHUNK_MAGIC;
		hdr.major = kt::MAJOR;
		hdr.minor = kt::MINOR;
		hdr.num_chunks = current_chunks.count();
		fptr.write(&hdr,sizeof(CurrentChunksHeader));

//		Out() << "sizeof(CurrentChunksHeader)" << sizeof(CurrentChunksHeader) << endl;
		Out() << "Saving " << current_chunks.count() << " chunk downloads" << endl;
		for (CurChunkItr i = current_chunks.begin();i != current_chunks.end();++i)
		{
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

		CurrentChunksHeader chdr;
		fptr.read(&chdr,sizeof(CurrentChunksHeader));
		if (chdr.magic != CURRENT_CHUNK_MAGIC)
		{
			Out() << "Warning : current_chunks file corrupted" << endl;
			return;
		}

		Out() << "Loading " << chdr.num_chunks  << " active chunk downloads" << endl;
		for (Uint32 i = 0;i < chdr.num_chunks;i++)
		{
			ChunkDownloadHeader hdr;
			// first read header
			fptr.read(&hdr,sizeof(ChunkDownloadHeader));
			Out() << "Loading chunk " << hdr.index << endl;
			
			if (!cman.getChunk(hdr.index) || current_chunks.contains(hdr.index))
			{
				Out() << "Illegal chunk " << hdr.index << endl;
				return;
			}
			Chunk* c = cman.getChunk(hdr.index);
			if (cman.prepareChunk(c))
			{
				ChunkDownload* cd = new ChunkDownload(c);
				current_chunks.insert(hdr.index,cd);
				cd->load(fptr,hdr);
				downloaded += cd->bytesDownloaded();
			
				if (tmon)
					tmon->downloadStarted(cd);
			}
		}
		
		// reset curr_chunks_downloaded to 0
		curr_chunks_dowloaded = 0;
	}
	
	Uint32 Downloader::getDownloadedBytesOfCurrentChunksFile(const QString & file)
	{
		// Load all partial downloads
		File fptr;
		if (!fptr.open(file,"rb"))
			return 0;

		// read the number of chunks
		CurrentChunksHeader chdr;
		fptr.read(&chdr,sizeof(CurrentChunksHeader));
		if (chdr.magic != CURRENT_CHUNK_MAGIC)
		{
			Out() << "Warning : current_chunks file corrupted" << endl;
			return 0;
		}
		Uint32 num_bytes = 0;
	
		// load all chunks and calculate how much is downloaded
		for (Uint32 i = 0;i < chdr.num_chunks;i++)
		{
			// read the chunkdownload header
			ChunkDownloadHeader hdr;
			fptr.read(&hdr,sizeof(ChunkDownloadHeader));
			
			Chunk* c = cman.getChunk(hdr.index);
			if (!c)
				return num_bytes;
			
			Uint32 last_size = c->getSize() % MAX_PIECE_LEN;
			if (last_size == 0)
				last_size = MAX_PIECE_LEN;
			
			// create the bitset and read it 
			BitSet bs(hdr.num_bits);
			fptr.read(bs.getData(),bs.getNumBytes());
			
			for (Uint32 j = 0;j < hdr.num_bits;j++)
			{
				if (bs.get(j))
					num_bytes += j == hdr.num_bits - 1 ? 
							last_size : MAX_PIECE_LEN;
			}
			
			if (hdr.buffered)
				fptr.seek(File::CURRENT,c->getSize());
		}
		curr_chunks_dowloaded = num_bytes;
		return num_bytes;
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
			
			if (!cd->isIdle())
				if (num_non_idle > 0) num_non_idle--;
			
			cd->cancelAll();
			Chunk* c = cd->getChunk();
			if (tmon)
				tmon->downloadRemoved(cd);
			current_chunks.erase(i);
			
			if (c->getStatus() == Chunk::MMAPPED)
				cman.saveChunk(i,false);
		}
	}
	
	Uint32 Downloader::mem_usage = 0;
	
	void Downloader::setMemoryUsage(Uint32 m)
	{
		mem_usage = m;
		PeerDownloader::setMemoryUsage(m);
	}
}
#include "downloader.moc"
