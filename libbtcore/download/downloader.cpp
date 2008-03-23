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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include "downloader.h"
#include <util/file.h>
#include <util/log.h>
#include <diskio/chunkmanager.h>
#include <torrent/torrent.h>
#include <peer/peermanager.h>
#include <util/error.h>
#include "chunkdownload.h"
#include <util/sha1hash.h>
#include <util/array.h>
#include <peer/peer.h>
#include <download/piece.h>
#include <peer/peerdownloader.h>
#include <util/functions.h>
#include <interfaces/monitorinterface.h>
#include <peer/packetwriter.h>
#include <torrent/ipblocklist.h>
#include "chunkselector.h"
#include "btversion.h"
#include "webseed.h"

namespace bt
{
	
	

	Downloader::Downloader(Torrent & tor,PeerManager & pman,ChunkManager & cman,ChunkSelectorFactoryInterface* fac) 
	: tor(tor),pman(pman),cman(cman),downloaded(0),tmon(0),chunk_selector(0)
	{
		if (!fac) // check if a custom one was provided, if not create a default one
			chunk_selector = new ChunkSelector(cman,*this,pman);
		else
			chunk_selector = fac->createChunkSelector(cman,*this,pman);
		
		Uint64 total = tor.getFileLength();
		downloaded = (total - cman.bytesLeft());
		curr_chunks_downloaded = 0;
		unnecessary_data = 0;
	
		current_chunks.setAutoDelete(true);
		connect(&pman,SIGNAL(newPeer(Peer* )),this,SLOT(onNewPeer(Peer* )));
		connect(&pman,SIGNAL(peerKilled(Peer* )),this,SLOT(onPeerKilled(Peer*)));
		
		active_webseed_downloads = 0;
		const KUrl::List & urls = tor.getWebSeeds();
		foreach (KUrl u,urls)
		{
			if (u.protocol() == "http")
			{
				WebSeed* ws = new WebSeed(u,tor,cman);
				webseeds.append(ws);
				connect(ws,SIGNAL(chunkReady(Chunk*)),this,SLOT(onChunkReady(Chunk*)));
				connect(ws,SIGNAL(chunkDownloadStarted(ChunkDownloadInterface*)),
						this,SLOT(chunkDownloadStarted(ChunkDownloadInterface*)));
				connect(ws,SIGNAL(chunkDownloadFinished(ChunkDownloadInterface*)),
						this,SLOT(chunkDownloadFinished(ChunkDownloadInterface*)));
			}
		}
	}


	Downloader::~Downloader()
	{
		delete chunk_selector;
		qDeleteAll(webseeds);
	}
	
	void Downloader::pieceRecieved(const Piece & p)
	{
		if (cman.completed())
			return;
		
		ChunkDownload* cd = 0;
		
		for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();++j)
		{
			if (p.getIndex() != j->first)
				continue;
			
			cd = j->second;
			break;
		}
		
		if (!cd)
		{
			unnecessary_data += p.getLength();
			Out(SYS_DIO|LOG_DEBUG) << 
					"Unnecessary piece, total unnecessary data : " << BytesToString(unnecessary_data) << endl;
			return;
		}
		
		// if the chunk is not in memory, reload it
		if (cd->getChunk()->getStatus() == Chunk::ON_DISK)
		{
			cman.prepareChunk(cd->getChunk(),true);
		}
			
		bool ok = false;
		
		if (cd->piece(p,ok))
		{
			if (tmon)
				tmon->downloadRemoved(cd);
				
			if (ok)
				downloaded += p.getLength();
			
			if (!finished(cd))
			{
				// if the chunk fails don't count the bytes downloaded
				if (cd->getChunk()->getSize() > downloaded)
					downloaded = 0;
				else
					downloaded -= cd->getChunk()->getSize();
			}
			current_chunks.erase(p.getIndex());
			update(); // run an update to assign new pieces
		}
		else
		{
			if (ok)
				downloaded += p.getLength();
			
			// save to disk again, if it is idle
			if (cd->isIdle() && cd->getChunk()->getStatus() == Chunk::MMAPPED)
			{
				cman.saveChunk(cd->getChunk()->getIndex(),false);
			}
		}
			
		if (!ok)
		{
			unnecessary_data += p.getLength();
			Out(SYS_DIO|LOG_DEBUG) << 
					"Unnecessary piece, total unnecessary data : " << BytesToString(unnecessary_data) << endl; 
		}
	}
	
	void Downloader::update()
	{
		if (cman.completed())
			return;
		
		/*
			Normal update should now handle all modes properly.
		*/
		normalUpdate();
		
		// now see if there aren't any timed out pieces
		foreach (PieceDownloader* pd,piece_downloaders)
		{
			pd->checkTimeouts();
		}
		
		
		foreach (WebSeed* ws,webseeds)
		{
			downloaded += ws->update();
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
				if (c->getStatus() == Chunk::MMAPPED && !webseeds_chunks.find(c->getIndex()))
				{
					cman.saveChunk(c->getIndex(),false);
				}
			} 
			else if (cd->isChoked())
			{
				cd->releaseAllPDs();
				Chunk* c = cd->getChunk();
				if (c->getStatus() == Chunk::MMAPPED && !webseeds_chunks.find(c->getIndex()))
				{
					cman.saveChunk(c->getIndex(),false);
				}
			}
			else if (cd->needsToBeUpdated())
			{
				cd->update();
			}
		}
		
		foreach (PieceDownloader* pd,piece_downloaders)
		{
			if (pd->canDownloadChunk())
			{
				if (!pd->isChoked())
					downloadFrom(pd);
				
				pd->setNearlyDone(false);
			}
		}
		
		foreach (WebSeed* ws,webseeds)
		{
			if (!ws->busy())
			{
				downloadFrom(ws);
			}
		}
	}

	Uint32 Downloader::maxMemoryUsage()
	{
		Uint32 max = 1024 * 1024;
		switch (mem_usage)
		{	
			case 1: // Medium
				max *= 60; // 60 MB
				break;
			case 2: // High
				max *= 80; // 90 MB
				break;
			case 0: // LOW
			default:
				max *= 40; // 30 MB
				break;
		}
		return max;
	}
	
	Uint32 Downloader::numNonIdle()
	{
		Uint32 num_non_idle = 0;
		for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();++j)
		{
			ChunkDownload* cd = j->second;
			if (!cd->isIdle())
				num_non_idle++;
		}
		return num_non_idle;
	}
	
	ChunkDownload* Downloader::selectCD(PieceDownloader* pd,Uint32 num)
	{
		ChunkDownload* sel = 0;
		Uint32 sel_left = 0xFFFFFFFF;
		
		for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();++j)
		{
			ChunkDownload* cd = j->second;
			if (pd->isChoked() || !pd->hasChunk(cd->getChunk()->getIndex()))
				continue;
			
			if (cd->getNumDownloaders() == num) 
			{
				// lets favor the ones which are nearly finished
				if (!sel || cd->getTotalPieces() - cd->getPiecesDownloaded() < sel_left)
				{
					sel = cd;
					sel_left = sel->getTotalPieces() - sel->getPiecesDownloaded();
				}
			}
		}
		return sel;
	}
	

	bool Downloader::findDownloadForPD(PieceDownloader* pd,bool warmup)
	{
		ChunkDownload* sel = 0;
		
		// first see if there are ChunkDownload's which need a PieceDownloader
		sel = selectCD(pd,0);
		
		if (!sel && warmup)
		{
			// if we couldn't find one, try to select another 
			// which only has one downloader
			// so that during warmup, there are at the most 2 downloaders 
			// assigned to one peer	
			sel = selectCD(pd,1);
		}
		
		if (sel)
		{
			// if it is on disk, reload it
			if (sel->getChunk()->getStatus() == Chunk::ON_DISK)
				cman.prepareChunk(sel->getChunk(),true);
			
			sel->assign(pd);
			return true;
		}
		
		return false;
	}
	
	ChunkDownload* Downloader::selectWorst(PieceDownloader* pd)
	{
		ChunkDownload* cdmin = NULL;
		for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();++j) 
		{ 
			ChunkDownload* cd = j->second; 
			if (!pd->hasChunk(cd->getChunk()->getIndex()) || cd->containsPeer(pd))
				continue;
			 
			if (!cdmin) 
				cdmin = cd;
			else if (cd->getDownloadSpeed() < cdmin->getDownloadSpeed())
				cdmin = cd;
			else if (cd->getNumDownloaders() < cdmin->getNumDownloaders()) 
				cdmin = cd; 
		}
		 
		return cdmin;
	}

	void Downloader::downloadFrom(PieceDownloader* pd)
	{
		// calculate the max memory usage
		Uint32 max = maxMemoryUsage();
		// calculate number of non idle chunks
		Uint32 num_non_idle = numNonIdle();
		
		// first see if we can use an existing dowload
		if (findDownloadForPD(pd,cman.getNumChunks() - cman.chunksLeft() <= 4))
			return;
		
		bool limit_exceeded = num_non_idle * tor.getChunkSize() >= max;
		
		Uint32 chunk = 0;
		if (!limit_exceeded && chunk_selector->select(pd,chunk))
		{
			Chunk* c = cman.getChunk(chunk);
			if (current_chunks.contains(chunk))
			{
				if (c->getStatus() == Chunk::ON_DISK)
					cman.prepareChunk(c,true);
			
				current_chunks.find(chunk)->assign(pd);
			}
			else if (cman.prepareChunk(c))
			{
				ChunkDownload* cd = new ChunkDownload(c);
				current_chunks.insert(chunk,cd);
				cd->assign(pd);
				if (tmon)
					tmon->downloadStarted(cd);
			}
		}
		else if (pd->getNumGrabbed() == 0)
		{ 
			// If the peer hasn't got a chunk we want, 
			ChunkDownload *cdmin = selectWorst(pd); 
			
			if (cdmin) 
			{
				// if it is on disk, reload it
				if (cdmin->getChunk()->getStatus() == Chunk::ON_DISK)
				{
					cman.prepareChunk(cdmin->getChunk(),true);
				}
				
				cdmin->assign(pd); 
			}
		} 
	}
	
	void Downloader::downloadFrom(WebSeed* ws)
	{
		Uint32 first = 0;
		Uint32 last = 0;
		if (chunk_selector->selectRange(first,last))
		{
			for (Uint32 i = first;i <= last;i++)
			{
				cman.prepareChunk(cman.getChunk(i),true);
				webseeds_chunks.insert(i,ws);
			}
			ws->download(first,last);
		}
	}
	

	bool Downloader::areWeDownloading(Uint32 chunk) const
	{
		return current_chunks.find(chunk) != 0 || webseeds_chunks.find(chunk) != 0;
	}
	
	bool Downloader::canDownloadFromWebSeed(Uint32 chunk) const
	{
		if (cman.chunksLeft() <= current_chunks.count() + webseeds_chunks.count())
			return true;
		else
			return !areWeDownloading(chunk);
	}
	
	Uint32 Downloader::numDownloadersForChunk(Uint32 chunk) const
	{
		const ChunkDownload* cd = current_chunks.find(chunk);
		if (!cd)
			return 0;
		
		return cd->getNumDownloaders();
	}
	
	void Downloader::onNewPeer(Peer* peer)
	{		
		PieceDownloader* pd = peer->getPeerDownloader();
		connect(pd,SIGNAL(downloaded(const bt::Piece& )),
				this,SLOT(pieceRecieved(const bt::Piece& )));
		piece_downloaders.append(pd);
	}

	void Downloader::onPeerKilled(Peer* peer)
	{
		PieceDownloader* pd = peer->getPeerDownloader();
		if (pd)
		{
			for (CurChunkItr i = current_chunks.begin();i != current_chunks.end();++i)
			{
				ChunkDownload* cd = i->second;
				cd->killed(pd);
			}
			piece_downloaders.removeAll(pd);
		}
	}
	
	bool Downloader::finished(ChunkDownload* cd)
	{
		Chunk* c = cd->getChunk();
		// verify the data
		SHA1Hash h;
		if (cd->usingContinuousHashing())
			h = cd->getHash();
		else
			h = SHA1Hash::generate(c->getData(),c->getSize());

		if (tor.verifyHash(h,c->getIndex()))
		{
			// hash ok so save it
			try
			{
				cman.saveChunk(c->getIndex());
				Out(SYS_GEN|LOG_IMPORTANT) << "Chunk " << c->getIndex() << " downloaded " << endl;
				// tell everybody we have the Chunk
				for (Uint32 i = 0;i < pman.getNumConnectedPeers();i++)
				{
					pman.getPeer(i)->getPacketWriter().sendHave(c->getIndex());
				}
			}
			catch (Error & e)
			{
				Out(SYS_DIO|LOG_IMPORTANT) << "Error " << e.toString() << endl;
				emit ioError(e.toString());
				return false;
			}
		}
		else
		{
			Out(SYS_GEN|LOG_IMPORTANT) << "Hash verification error on chunk "  << c->getIndex() << endl;
			Out(SYS_GEN|LOG_IMPORTANT) << "Is        : " << h << endl;
			Out(SYS_GEN|LOG_IMPORTANT) << "Should be : " << tor.getHash(c->getIndex()) << endl;
			
			// reset chunk but only when no webseeder is downloading it
			if (!webseeds_chunks.find(c->getIndex()))
				cman.resetChunk(c->getIndex());
			
			chunk_selector->reinsert(c->getIndex());

			PieceDownloader* only = cd->getOnlyDownloader();
			if (only)
			{
				Peer* p = pman.findPeer(only);
				if (!p)
					return false;
				QString IP(p->getIPAddresss());
				Out(SYS_GEN|LOG_NOTICE) << "Peer " << IP << " sent bad data" << endl;
				IPBlocklist & ipfilter = IPBlocklist::instance();
				ipfilter.insert( IP );
				p->kill(); 
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
		piece_downloaders.clear();
		
		foreach (WebSeed* ws,webseeds)
			ws->reset();
	}
	
	Uint32 Downloader::downloadRate() const
	{
		// sum of the download rate of each peer
		Uint32 rate = 0;
		foreach (PieceDownloader* pd,piece_downloaders)
			if (pd)
				rate += pd->getDownloadRate();
		
		
		foreach (WebSeed* ws,webseeds)
		{
			rate += ws->getDownloadRate();
		}
			
		return rate;
	}
	
	void Downloader::setMonitor(MonitorInterface* tmo)
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
		hdr.major = bt::MAJOR;
		hdr.minor = bt::MINOR;
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
		if (cman.completed())
			return;
		
		// Load all partial downloads
		File fptr;
		if (!fptr.open(file,"rb"))
			return;

		// recalculate downloaded bytes
		downloaded = (tor.getFileLength() - cman.bytesLeft());

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
			if (hdr.index >= tor.getNumChunks())
			{
				Out() << "Warning : current_chunks file corrupted, invalid index " << hdr.index << endl;
				return;
			}
			
			if (!cman.getChunk(hdr.index) || current_chunks.contains(hdr.index))
			{
				Out() << "Illegal chunk " << hdr.index << endl;
				return;
			}
			Chunk* c = cman.getChunk(hdr.index);
			if (!c->isExcluded() && cman.prepareChunk(c))
			{
				ChunkDownload* cd = new ChunkDownload(c);
				bool ret = false;
				try
				{
					ret = cd->load(fptr,hdr);
				}
				catch (...)
				{
					ret = false;
				}
				
				if (!ret)
				{
					delete cd;
				}
				else
				{
					current_chunks.insert(hdr.index,cd);
					downloaded += cd->bytesDownloaded();
			
					if (tmon)
						tmon->downloadStarted(cd);
				}
			}
		}
		
		// reset curr_chunks_downloaded to 0
		curr_chunks_downloaded = 0;
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
		curr_chunks_downloaded = num_bytes;
		return num_bytes;
	}

	bool Downloader::isFinished() const
	{
		return cman.completed();
	}

	void Downloader::onExcluded(Uint32 from,Uint32 to)
	{
		for (Uint32 i = from;i <= to;i++)
		{
			ChunkDownload* cd = current_chunks.find(i);
			// finish only seed chunks
			if (!cd || cman.getChunk(i)->getPriority() == bt::ONLY_SEED_PRIORITY)
				continue;
			
			cd->cancelAll();
			cd->releaseAllPDs();
			if (tmon)
				tmon->downloadRemoved(cd);
			current_chunks.erase(i);
			cman.resetChunk(i); // reset chunk it is not fully downloaded yet
		}
		
		foreach (WebSeed* ws,webseeds)
		{
			ws->onExcluded(from,to);
		}
	}
	
	void Downloader::onIncluded(Uint32 from,Uint32 to)
	{
		chunk_selector->reincluded(from,to);
	}
	
	void Downloader::corrupted(Uint32 chunk)
	{
		chunk_selector->reinsert(chunk);
	}
	
	Uint32 Downloader::mem_usage = 0;
	
	void Downloader::setMemoryUsage(Uint32 m)
	{
		mem_usage = m;
	}
	
	void Downloader::dataChecked(const BitSet & ok_chunks)
	{
		for (Uint32 i = 0;i < ok_chunks.getNumBits();i++)
		{
			ChunkDownload* cd = current_chunks.find(i);
			if (ok_chunks.get(i) && cd)
			{
				// we have a chunk and we are downloading it so kill it
				cd->releaseAllPDs();
				if (tmon)
					tmon->downloadRemoved(cd);
				
				current_chunks.erase(i);
			}
		}
		chunk_selector->dataChecked(ok_chunks);
	}
	
	void Downloader::recalcDownloaded()
	{
		Uint64 total = tor.getFileLength();
		downloaded = (total - cman.bytesLeft());
	}
	
	void Downloader::onChunkReady(Chunk* c)
	{
		SHA1Hash h = SHA1Hash::generate(c->getData(),c->getSize());
		webseeds_chunks.erase(c->getIndex());
		
		if (tor.verifyHash(h,c->getIndex()))
		{
			// hash ok so save it
			try
			{
				ChunkDownload* cd = current_chunks.find(c->getIndex());
				if (cd)
				{
					// A ChunkDownload is ongoing for this chunk so kill it, we have the chunk
					cd->cancelAll();
					current_chunks.erase(c->getIndex());
				}
				
				cman.saveChunk(c->getIndex());
			
				Out(SYS_GEN|LOG_IMPORTANT) << "Chunk " << c->getIndex() << " downloaded via webseed ! " << endl;
				// tell everybody we have the Chunk
				for (Uint32 i = 0;i < pman.getNumConnectedPeers();i++)
				{
					pman.getPeer(i)->getPacketWriter().sendHave(c->getIndex());
				}
			}
			catch (Error & e)
			{
				Out(SYS_DIO|LOG_IMPORTANT) << "Error " << e.toString() << endl;
				emit ioError(e.toString());
			}
		}
		else
		{
			Out(SYS_GEN|LOG_IMPORTANT) << "Hash verification error on chunk "  << c->getIndex() << endl;
			Out(SYS_GEN|LOG_IMPORTANT) << "Is        : " << h << endl;
			Out(SYS_GEN|LOG_IMPORTANT) << "Should be : " << tor.getHash(c->getIndex()) << endl;
		
			// reset chunk but only when no other peer is downloading it
			if (!current_chunks.find(c->getIndex()))
				cman.resetChunk(c->getIndex());
			
			chunk_selector->reinsert(c->getIndex());
		}
	}
	
	void Downloader::chunkDownloadStarted(ChunkDownloadInterface* cd)
	{
		active_webseed_downloads++;
		if (tmon)
			tmon->downloadStarted(cd);
	}
	
	void Downloader::chunkDownloadFinished(ChunkDownloadInterface* cd)
	{
		if (active_webseed_downloads > 0)
			active_webseed_downloads--;
		
		if (tmon)
			tmon->downloadRemoved(cd);
	}
}

#include "downloader.moc"
