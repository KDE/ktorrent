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

#include <QFile>
#include <QTextStream>
#include <util/file.h>
#include <util/log.h>
#include <diskio/chunkmanager.h>
#include <diskio/piecedata.h>
#include <torrent/torrent.h>
#include <peer/peermanager.h>
#include <util/error.h>
#include "chunkdownload.h"
#include <util/sha1hash.h>
#include <util/array.h>
#include <peer/peer.h>
#include <download/piece.h>
#include <peer/peerdownloader.h>
#include <peer/badpeerslist.h>
#include <util/functions.h>
#include <interfaces/monitorinterface.h>
#include <peer/packetwriter.h>
#include <peer/accessmanager.h>
#include "chunkselector.h"
#include "btversion.h"
#include "webseed.h"


namespace bt
{

	bool Downloader::use_webseeds = true;
	
	Downloader::Downloader(Torrent & tor,PeerManager & pman,ChunkManager & cman) 
	: tor(tor),pman(pman),cman(cman),downloaded(0),tmon(0),chunk_selector(0),webseed_endgame_mode(false)
	{
		webseeds_on = use_webseeds;
		pman.setPieceHandler(this);
		chunk_selector = new ChunkSelector(cman,*this,pman);
		
		Uint64 total = tor.getTotalSize();
		downloaded = (total - cman.bytesLeft());
		curr_chunks_downloaded = 0;
		unnecessary_data = 0;
	
		current_chunks.setAutoDelete(true);
		connect(&pman,SIGNAL(newPeer(Peer* )),this,SLOT(onNewPeer(Peer* )));
		connect(&pman,SIGNAL(peerKilled(Peer* )),this,SLOT(onPeerKilled(Peer*)));
		
		active_webseed_downloads = 0;
		const KUrl::List & urls = tor.getWebSeeds();
		foreach (const KUrl &u,urls)
		{
			if (u.protocol() == "http")
			{
				WebSeed* ws = new WebSeed(u,false,tor,cman);
				webseeds.append(ws);
				connect(ws,SIGNAL(chunkReady(Chunk*)),this,SLOT(onChunkReady(Chunk*)));
				connect(ws,SIGNAL(chunkDownloadStarted(WebSeedChunkDownload*,Uint32)),
						this,SLOT(chunkDownloadStarted(WebSeedChunkDownload*,Uint32)));
				connect(ws,SIGNAL(chunkDownloadFinished(WebSeedChunkDownload*,Uint32)),
						this,SLOT(chunkDownloadFinished(WebSeedChunkDownload*,Uint32)));
			}
		}
		
		if (webseeds.count() > 0)
		{
			webseed_range_size = tor.getNumChunks() / webseeds.count();
			if (webseed_range_size == 0)
				webseed_range_size = 1;
			
			// make sure the range is not to big
			if (webseed_range_size > tor.getNumChunks() / 10)
				webseed_range_size = tor.getNumChunks() / 10;
		}
		else
		{
			webseed_range_size = 1;
		}
	}


	Downloader::~Downloader()
	{
		delete chunk_selector;
		qDeleteAll(webseeds);
	}
	
	void Downloader::setChunkSelector(ChunkSelectorInterface* csel)
	{
		if (chunk_selector)
			delete chunk_selector;
		
		if (!csel) // check if a custom one was provided, if not create a default one
			chunk_selector = new ChunkSelector(cman,*this,pman);
		else
			chunk_selector = csel;
	}

	
	void Downloader::pieceReceived(const Piece & p)
	{
		if (cman.completed())
			return;
		
		ChunkDownload* cd = current_chunks.find(p.getIndex());
		if (!cd)
		{
			unnecessary_data += p.getLength();
			Out(SYS_DIO|LOG_DEBUG) << 
					"Unnecessary piece, total unnecessary data : " << BytesToString(unnecessary_data) << endl;
			return;
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
				current_chunks.erase(p.getIndex());
			}
			else
			{
				current_chunks.erase(p.getIndex());
				foreach (WebSeed* ws,webseeds)
				{
					if (ws->inCurrentRange(p.getIndex()))
						ws->chunkDownloaded(p.getIndex());
				}
			}
		}
		else
		{
			if (ok)
				downloaded += p.getLength();
		}
			
		if (!ok)
		{
			unnecessary_data += p.getLength();
			Out(SYS_DIO|LOG_DEBUG) << 
					"Unnecessary piece, total unnecessary data : " << BytesToString(unnecessary_data) << endl; 
		}
	}
	
	bool Downloader::endgameMode() const
	{
		return current_chunks.count() >= cman.chunksLeft();
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
		
		if (use_webseeds)
		{
			foreach (WebSeed* ws,webseeds)
			{
				ws->update();
			}
		}
		
		if (isFinished() && webseeds_on)
		{
			foreach (WebSeed* ws,webseeds)
			{
				ws->cancel();
			}
		}
	}

	
	void Downloader::normalUpdate()
	{
		for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();++j)
		{
			ChunkDownload* cd = j->second;
			if (cd->isIdle())
			{
				continue;
			}
			else if (cd->isChoked())
			{
				cd->releaseAllPDs();
			}
			else if (cd->needsToBeUpdated())
			{
				cd->update();
			}
		}
		
		foreach (PieceDownloader* pd,piece_downloaders)
		{
			if (!pd->isChoked())
			{
				while (pd->canDownloadChunk())
				{
					if (!downloadFrom(pd))
						break;
					pd->setNearlyDone(false);
				}
			}
		}
		
		if (use_webseeds)
		{
			foreach (WebSeed* ws,webseeds)
			{
				if (!ws->busy() && ws->isEnabled() && ws->failedAttempts() < 3)
				{
					downloadFrom(ws);
				}
			}
		}
		else if (webseeds_on != use_webseeds)
		{
			// reset all webseeds, webseeds have been disabled
			webseeds_on = use_webseeds;
			foreach (WebSeed* ws,webseeds)
			{
				if (ws->busy() && ws->isEnabled())
				{
					ws->cancel();
				}
			}
		}
	}

	
	ChunkDownload* Downloader::selectCD(PieceDownloader* pd,Uint32 n)
	{
		ChunkDownload* sel = 0;
		Uint32 sel_left = 0xFFFFFFFF;
		
		for (CurChunkItr j = current_chunks.begin();j != current_chunks.end();++j)
		{
			ChunkDownload* cd = j->second;
			if (pd->isChoked() || !pd->hasChunk(cd->getChunk()->getIndex()))
				continue;
			
			if (cd->getNumDownloaders() == n) 
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
	

	bool Downloader::findDownloadForPD(PieceDownloader* pd)
	{
		ChunkDownload* sel = 0;
		
		// See if there are ChunkDownload's which need a PieceDownloader
		sel = selectCD(pd,0);
		if (sel)
		{
			return sel->assign(pd);
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

	bool Downloader::downloadFrom(PieceDownloader* pd)
	{
		// first see if we can use an existing dowload
		if (findDownloadForPD(pd))
			return true;
		
		Uint32 chunk = 0;
		if (chunk_selector->select(pd,chunk))
		{
			Chunk* c = cman.getChunk(chunk);
			if (current_chunks.contains(chunk))
			{
				return current_chunks.find(chunk)->assign(pd);
			}
			else
			{
				ChunkDownload* cd = new ChunkDownload(c);
				current_chunks.insert(chunk,cd);
				cd->assign(pd);
				if (tmon)
					tmon->downloadStarted(cd);
				return true;
			}
		}
		else if (pd->getNumGrabbed() == 0)
		{ 
			// If the peer hasn't got a chunk we want, 
			ChunkDownload *cdmin = selectWorst(pd); 
			
			if (cdmin) 
			{
				return cdmin->assign(pd); 
			}
		} 
		
		return false;
	}
	
	void Downloader::downloadFrom(WebSeed* ws)
	{
		Uint32 first = 0;
		Uint32 last = 0;
		webseed_endgame_mode = false;
		if (chunk_selector->selectRange(first,last,webseed_range_size))
		{
			ws->download(first,last);
		}
		else
		{
			// go to endgame mode
			webseed_endgame_mode = true;
			if (chunk_selector->selectRange(first,last,webseed_range_size))
				ws->download(first,last);
		}
	}
	

	bool Downloader::areWeDownloading(Uint32 chunk) const
	{
		return current_chunks.find(chunk) != 0;
	}
	
	bool Downloader::canDownloadFromWebSeed(Uint32 chunk) const
	{
		if (webseed_endgame_mode)
			return true;
		
		foreach (WebSeed* ws,webseeds)
		{
			if (ws->busy() && ws->inCurrentRange(chunk))
				return false;
		}
		
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
		SHA1Hash h = cd->getHash();
		
		if (tor.verifyHash(h,c->getIndex()))
		{
			// hash ok so save it
			try
			{
				foreach (WebSeed* ws,webseeds)
				{
					// tell all webseeds a chunk is downloaded
					if (ws->inCurrentRange(c->getIndex()))
						ws->chunkDownloaded(c->getIndex());
				}
				
				cman.chunkDownloaded(c->getIndex());
				Out(SYS_GEN|LOG_IMPORTANT) << "Chunk " << c->getIndex() << " downloaded " << endl;
				// tell everybody we have the Chunk
				for (Uint32 i = 0;i < pman.getNumConnectedPeers();i++)
				{
					pman.getPeer(i)->getPacketWriter().sendHave(c->getIndex());
				}
				
				emit chunkDownloaded(c->getIndex());
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
				
				QString ip = p->getIPAddresss();
				Out(SYS_GEN|LOG_NOTICE) << "Peer " << ip << " sent bad data" << endl;
				AccessManager::instance().banPeer(ip);
				p->kill(); 
			}
			return false;
		}
		return true;
	}
	
	void Downloader::clearDownloads()
	{
		current_chunks.clear();
		piece_downloaders.clear();
		
		foreach (WebSeed* ws,webseeds)
			ws->cancel();
	}
	
	void Downloader::pause()
	{
		if (tmon)
		{
			for (CurChunkItr i = current_chunks.begin();i != current_chunks.end();++i)
			{
				ChunkDownload* cd = i->second;
				tmon->downloadRemoved(cd);
			}
		}
		
		current_chunks.clear();
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
		
		foreach (WebSeed* ws,webseeds)
		{
			WebSeedChunkDownload* cd = ws->currentChunkDownload();
			if (cd)
				tmon->downloadStarted(cd);
		}
		
	}
	


	void Downloader::saveDownloads(const QString & file)
	{
		File fptr;
		if (!fptr.open(file,"wb"))
			return;

		// See bug 219019, don't know why, but it is possible that we get 0 pointers in the map
		// so get rid of them before we save
		for (CurChunkItr i = current_chunks.begin();i != current_chunks.end();)
		{
			if (!i->second)
				i = current_chunks.erase(i);
			else
				i++;
		}
		
		// Save all the current downloads to a file
		CurrentChunksHeader hdr;
		hdr.magic = CURRENT_CHUNK_MAGIC;
		hdr.major = bt::MAJOR;
		hdr.minor = bt::MINOR;
		hdr.num_chunks = current_chunks.count();
		fptr.write(&hdr,sizeof(CurrentChunksHeader));
		
		Out(SYS_GEN|LOG_DEBUG) << "Saving " << current_chunks.count() << " chunk downloads" << endl;
		for (CurChunkItr i = current_chunks.begin();i != current_chunks.end();i++)
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
		downloaded = (tor.getTotalSize() - cman.bytesLeft());

		CurrentChunksHeader chdr;
		fptr.read(&chdr,sizeof(CurrentChunksHeader));
		if (chdr.magic != CURRENT_CHUNK_MAGIC)
		{
			Out(SYS_GEN|LOG_DEBUG) << "Warning : current_chunks file corrupted" << endl;
			return;
		}

		Out(SYS_GEN|LOG_DEBUG) << "Loading " << chdr.num_chunks  << " active chunk downloads" << endl;
		for (Uint32 i = 0;i < chdr.num_chunks;i++)
		{
			ChunkDownloadHeader hdr;
			// first read header
			fptr.read(&hdr,sizeof(ChunkDownloadHeader));
			Out(SYS_GEN|LOG_DEBUG) << "Loading chunk " << hdr.index << endl;
			if (hdr.index >= tor.getNumChunks())
			{
				Out(SYS_GEN|LOG_DEBUG) << "Warning : current_chunks file corrupted, invalid index " << hdr.index << endl;
				return;
			}
			
			Chunk* c = cman.getChunk(hdr.index);
			if (!c || current_chunks.contains(hdr.index))
			{
				Out(SYS_GEN|LOG_DEBUG) << "Illegal chunk " << hdr.index << endl;
				return;
			}
			
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
			
			if (!ret || c->getStatus() == Chunk::ON_DISK || c->isExcluded())
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
			Out(SYS_GEN|LOG_DEBUG) << "Warning : current_chunks file corrupted" << endl;
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
			
			ChunkDownload tmp(c);
			if (!tmp.load(fptr,hdr,false))
				return num_bytes;
			
			num_bytes += tmp.bytesDownloaded();
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
			if (!cd)
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
		Uint64 total = tor.getTotalSize();
		downloaded = (total - cman.bytesLeft());
	}
	
	void Downloader::onChunkReady(Chunk* c)
	{
		PieceDataPtr piece = c->getPiece(0,c->getSize(),false);
		
		webseeds_chunks.erase(c->getIndex());
		if (!piece)
		{
			// reset chunk but only when no other peer is downloading it
			if (!current_chunks.find(c->getIndex()))
				cman.resetChunk(c->getIndex());
			
			chunk_selector->reinsert(c->getIndex());
			return;
		}
		

		SHA1Hash h = SHA1Hash::generate(piece->data(),c->getSize());
		if (tor.verifyHash(h,c->getIndex()))
		{
			// hash ok so save it
			try
			{
				downloaded += c->getSize();
				
				foreach (WebSeed* ws,webseeds)
				{
					// tell all webseeds a chunk is downloaded
					if (ws->inCurrentRange(c->getIndex()))
						ws->chunkDownloaded(c->getIndex());
				}
				
				ChunkDownload* cd = current_chunks.find(c->getIndex());
				if (cd)
				{
					// A ChunkDownload is ongoing for this chunk so kill it, we have the chunk
					cd->cancelAll();
					current_chunks.erase(c->getIndex());
				}
				
				c->savePiece(piece);
				cman.chunkDownloaded(c->getIndex());
			
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
	
	void Downloader::chunkDownloadStarted(WebSeedChunkDownload* cd,Uint32 chunk)
	{
		webseeds_chunks.insert(chunk,cd->ws);
		active_webseed_downloads++;
		if (tmon)
			tmon->downloadStarted(cd);
	}
	
	void Downloader::chunkDownloadFinished(WebSeedChunkDownload* cd,Uint32 chunk)
	{
		webseeds_chunks.erase(chunk);
		if (active_webseed_downloads > 0)
			active_webseed_downloads--;
		
		if (tmon)
			tmon->downloadRemoved(cd);
	}
	
	WebSeed* Downloader::addWebSeed(const KUrl & url)
	{
		// Check for dupes
		foreach (WebSeed* ws,webseeds)
		{
			if (ws->getUrl() == url)
				return 0;
		}
		
		WebSeed* ws = new WebSeed(url,true,tor,cman);
		webseeds.append(ws);
		connect(ws,SIGNAL(chunkReady(Chunk*)),this,SLOT(onChunkReady(Chunk*)));
		connect(ws,SIGNAL(chunkDownloadStarted(WebSeedChunkDownload*,Uint32)),
				this,SLOT(chunkDownloadStarted(WebSeedChunkDownload*,Uint32)));
		connect(ws,SIGNAL(chunkDownloadFinished(WebSeedChunkDownload*,Uint32)),
				this,SLOT(chunkDownloadFinished(WebSeedChunkDownload*,Uint32)));
		return ws;
	}
		
	bool Downloader::removeWebSeed(const KUrl & url)
	{
		foreach (WebSeed* ws,webseeds)
		{
			if (ws->getUrl() == url && ws->isUserCreated())
			{
				PtrMap<Uint32,WebSeed>::iterator i = webseeds_chunks.begin();
				while (i != webseeds_chunks.end())
				{
					if (i->second == ws)
						i = webseeds_chunks.erase(i);
					else
						i++;
				}
				webseeds.removeAll(ws);
				delete ws;
				return true;
			}
		}
		return false;
	}
	
	void Downloader::removeAllWebSeeds()
	{
		webseeds.clear();
		webseeds_chunks.clear();
	}

	void Downloader::saveWebSeeds(const QString & file)
	{
		QFile fptr(file);
		if (!fptr.open(QIODevice::WriteOnly))
		{
			Out(SYS_GEN|LOG_NOTICE) << "Cannot open " << file << " to save webseeds" << endl;
			return;
		}
		
		QTextStream out(&fptr); 
		foreach (WebSeed* ws,webseeds)
		{
			if (ws->isUserCreated())
				out << ws->getUrl().prettyUrl() << ::endl;
		}
		out << "====disabled====" << ::endl;
		foreach (WebSeed* ws,webseeds)
		{
			if (!ws->isEnabled())
				out << ws->getUrl().prettyUrl() << ::endl;
		}
	}
	
	void Downloader::loadWebSeeds(const QString & file)
	{		
		QFile fptr(file);
		if (!fptr.open(QIODevice::ReadOnly))
		{
			Out(SYS_GEN|LOG_NOTICE) << "Cannot open " << file << " to load webseeds" << endl;
			return;
		}
		
		bool disabled_list_found = false;
		QTextStream in(&fptr); 
		while (!in.atEnd())
		{
			QString line = in.readLine();
			if (line == "====disabled====")
			{
				disabled_list_found = true;
				continue;
			}
			
			KUrl url(line);
			if (!url.isValid() || url.protocol() != "http")
				continue;
				
			if (disabled_list_found)
			{
				foreach (WebSeed* ws,webseeds)
				{
					if (ws->getUrl() == url)
					{
						ws->setEnabled(false);
						break;
					}
				}
			}
			else
			{
				WebSeed* ws = new WebSeed(url,true,tor,cman);
				webseeds.append(ws);
				connect(ws,SIGNAL(chunkReady(Chunk*)),this,SLOT(onChunkReady(Chunk*)));
				connect(ws,SIGNAL(chunkDownloadStarted(WebSeedChunkDownload*,Uint32)),
						this,SLOT(chunkDownloadStarted(WebSeedChunkDownload*,Uint32)));
				connect(ws,SIGNAL(chunkDownloadFinished(WebSeedChunkDownload*,Uint32)),
						this,SLOT(chunkDownloadFinished(WebSeedChunkDownload*,Uint32)));
			}
		}
	}
	
	void Downloader::setGroupIDs(Uint32 up,Uint32 down)
	{
		foreach (WebSeed* ws,webseeds)
		{
			ws->setGroupIDs(up,down);
		}
	}
	
	ChunkDownload* Downloader::getDownload(Uint32 chunk)
	{
		return current_chunks.find(chunk);
	}
	
	void Downloader::setUseWebSeeds(bool on) 
	{
		use_webseeds = on;
	}

}

#include "downloader.moc"
