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

#include <algorithm>
#include <util/file.h>
#include <util/log.h>
#include <util/array.h>
#include "chunkdownload.h"
#include "downloader.h"
#include "chunk.h"
#include "peer.h"
#include "peermanager.h"
#include "piece.h"
#include "peerdownloader.h"

#include <klocale.h>

namespace bt
{
	const Uint8 PIECE_NOT_DOWNLOADED = 0;
	const Uint8 PIECE_REQUESTED = 1;
	const Uint8 PIECE_DOWNLOADED = 2;
	
	

	class DownloadStatus
	{
		std::set<Uint32> requested_pieces;
	public:
		typedef std::set<Uint32>::iterator iterator;
		DownloadStatus()
		{
		}

		~DownloadStatus()
		{
		}

		void add(Uint32 p)
		{
			requested_pieces.insert(p);
		}
		
		void remove(Uint32 p)
		{
			requested_pieces.erase(p);
		}
		
		bool contains(Uint32 p)
		{
			return requested_pieces.count(p) > 0;
		}
		
		void clear()
		{
			requested_pieces.clear();
		}
		
		iterator begin() {return requested_pieces.begin();}
		iterator end() {return requested_pieces.end();}
	};
	
	ChunkDownload::ChunkDownload(Chunk* chunk) : chunk(chunk)
	{
		num = num_downloaded = 0;
		
		num = chunk->getSize() / MAX_PIECE_LEN;
		
		if (chunk->getSize() % MAX_PIECE_LEN != 0)
		{
			last_size = chunk->getSize() % MAX_PIECE_LEN;
			num++;
		}
		else
		{
			last_size = MAX_PIECE_LEN;
		}
		
		pieces = BitSet(num);
		pieces.clear();
		
		for (Uint32 i = 0;i < num;i++)
			piece_queue.append(i);
		
		dstatus.setAutoDelete(true);
		chunk->ref();
	}

	ChunkDownload::~ChunkDownload()
	{
		chunk->unref();
	}

	bool ChunkDownload::piece(const Piece & p)
	{
		timer.update();
		if (num_downloaded == num)
		{
		//	Out() << "num_downloaded == num" << endl;
			return true;
		}
			
		Uint32 pp = p.getOffset() / MAX_PIECE_LEN;
		if (pieces.get(pp))
		{
		//	Out() << "pieces[pp] == PIECE_DOWNLOADED" << endl;
			return false;
		}

	//	Out() << "ChunkDownload::piece " << chunk->getIndex() << endl;
	//	Out() << "Piece " << p.getIndex() << " " << p.getOffset() << " " << pp << endl;
		DownloadStatus* ds = dstatus.find(p.getPeer());
		Uint8* buf = chunk->getData();
		if (buf)
		{
			memcpy(buf + p.getOffset(),p.getData(),p.getLength());
			if (ds)
				ds->remove(pp);
			pieces.set(pp,true);
			piece_queue.remove(pp);
			piece_providers.insert(p.getPeer());
			num_downloaded++;
			
			if (pdown.count() > 1)
			{
				endgameCancel(p);
			}
		}
		
		if (num_downloaded == num)
		{
			releaseAllPDs();
			return true;
		}
		
		for (QPtrList<PeerDownloader>::iterator i = pdown.begin();i != pdown.end();++i)
			sendRequests(*i);

		return false;
	}
	
	void ChunkDownload::releaseAllPDs()
	{
		for (Uint32 i = 0;i < pdown.count();i++)
		{
			PeerDownloader* pd = pdown.at(i);
			pd->release();
			disconnect(pd,SIGNAL(timedout(const Request& )),this,SLOT(onTimeout(const Request& )));
		}
		dstatus.clear();
		pdown.clear();
	}
	
	bool ChunkDownload::assignPeer(PeerDownloader* pd)
	{
		if (!pd || pdown.contains(pd))
			return false;
			
		pd->grab();
		pdown.append(pd);
		dstatus.insert(pd->getPeer()->getID(),new DownloadStatus());
		sendRequests(pd);
		connect(pd,SIGNAL(timedout(const Request& )),this,SLOT(onTimeout(const Request& )));
		return true;
	}
	
	void ChunkDownload::sendRequests(PeerDownloader* pd)
	{
		timer.update();
		DownloadStatus* ds = dstatus.find(pd->getPeer()->getID());
		if (!ds)
			return;
		
		Uint32 max_outstanding = pd->getMaximumOutstandingReqs();
		Uint32 num_visited = 0;
		while (num_visited < num && pd->getNumRequests() < max_outstanding)
		{
			// get the first one in the queue
			Uint32 i = piece_queue.first();
			if (!ds->contains(i))
			{
				// send request
				pd->download(
						Request(
							chunk->getIndex(),
							i*MAX_PIECE_LEN,
							i+1<num ? MAX_PIECE_LEN : last_size,0));
				ds->add(i);
			}
			// move to the back so that it will take a while before it's turn is up
			piece_queue.pop_front();
			piece_queue.append(i);
			num_visited++;
		}
	}
	
	void ChunkDownload::onTimeout(const Request & r)
	{
		// see if we are dealing with a piece of ours
		if (chunk->getIndex() == r.getIndex())
		{
			// find the peer 
			DownloadStatus* ds = dstatus.find(r.getPeer());
			if (!ds)
				return;
			
			Uint32 p  = r.getOffset() / MAX_PIECE_LEN;
			ds->remove(p);
			
			// go over all PD's and do requets again
			for (QPtrList<PeerDownloader>::iterator i = pdown.begin();i != pdown.end();++i)
				sendRequests(*i);
		}
	}
	
	
	void ChunkDownload::sendCancels(PeerDownloader* pd)
	{
		DownloadStatus* ds = dstatus.find(pd->getPeer()->getID());
		if (!ds)
			return;
		
		DownloadStatus::iterator itr = ds->begin();
		while (itr != ds->end())
		{
			Uint32 i = *itr;
			pd->cancel(
					Request(
						chunk->getIndex(),
						i*MAX_PIECE_LEN,
						i+1<num ? MAX_PIECE_LEN : last_size,0));
			itr++;
		}
		ds->clear();
		timer.update();
	}
	
	void ChunkDownload::endgameCancel(const Piece & p)
	{
		QPtrList<PeerDownloader>::iterator i = pdown.begin();
		while (i != pdown.end())
		{
			PeerDownloader* pd = *i;
			DownloadStatus* ds = dstatus.find(pd->getPeer()->getID());
			Uint32 pp = p.getOffset() / MAX_PIECE_LEN;
			if (ds && ds->contains(pp))
			{
				pd->cancel(Request(p));
				ds->remove(pp);
			}
			i++;
		}
	}

	void ChunkDownload::peerKilled(PeerDownloader* pd)
	{
		if (!pdown.contains(pd))
			return;

		dstatus.erase(pd->getPeer()->getID());
		pdown.remove(pd);
		disconnect(pd,SIGNAL(timedout(const Request& )),this,SLOT(onTimeout(const Request& )));
	}
	
	
	const Peer* ChunkDownload::getCurrentPeer() const
	{
		if (pdown.count() == 0)
			return 0;
		else
			return pdown.getFirst()->getPeer();
	}
	
	Uint32 ChunkDownload::getChunkIndex() const
	{
		return chunk->getIndex();
	}

	QString ChunkDownload::getCurrentPeerID() const
	{
		if (pdown.count() == 0)
		{
			return QString::null;
		}
		else if (pdown.count() == 1)
		{
			const Peer* p = pdown.getFirst()->getPeer();
			return p->getPeerID().identifyClient();
		}
		else
		{
			return i18n("1 peer","%n peers",pdown.count());
		}
	}

	Uint32 ChunkDownload::getDownloadSpeed() const
	{
		Uint32 r = 0;
		QPtrList<PeerDownloader>::const_iterator i = pdown.begin();
		while (i != pdown.end())
		{
			const PeerDownloader* pd = *i;
			r += pd->getPeer()->getDownloadRate();
			i++;
		}
		return r;
	}
	


	void ChunkDownload::save(File & file)
	{	
		ChunkDownloadHeader hdr;
		hdr.index = chunk->getIndex();
		hdr.num_bits = pieces.getNumBits();
		hdr.buffered = chunk->getStatus() == Chunk::BUFFERED ? 1 : 0;
		// save the chunk header
		file.write(&hdr,sizeof(ChunkDownloadHeader));
		// save the bitset
		file.write(pieces.getData(),pieces.getNumBytes());
		if (hdr.buffered)
		{
			// if it's a buffered chunk, save the contents to
			file.write(chunk->getData(),chunk->getSize());
			chunk->clear();
			chunk->setStatus(Chunk::ON_DISK);
		}
	}
		
	void ChunkDownload::load(File & file,ChunkDownloadHeader & hdr)
	{
		// read pieces
		pieces = BitSet(hdr.num_bits);
		Array<Uint8> data(pieces.getNumBytes());
		file.read(data,pieces.getNumBytes());
		pieces = BitSet(data,hdr.num_bits);
		num_downloaded = pieces.numOnBits();
		if (hdr.buffered)
		{
			// if it's a buffered chunk, load the data to
			file.read(chunk->getData(),chunk->getSize());
		}
		
		for (Uint32 i = 0;i < pieces.getNumBits();i++)
			if (pieces.get(i))
				piece_queue.remove(i);
	}

	Uint32 ChunkDownload::bytesDownloaded() const
	{
		Uint32 num_bytes = 0;
		for (Uint32 i = 0;i < num;i++)
		{
			if (pieces.get(i))
			{
				num_bytes += i == num-1 ? last_size : MAX_PIECE_LEN;
			}
		}
		return num_bytes;
	}

	void ChunkDownload::cancelAll()
	{
		QPtrList<PeerDownloader>::iterator i = pdown.begin();
		while (i != pdown.end())
		{
			sendCancels(*i);
			i++;
		}
	}

	bool ChunkDownload::getOnlyDownloader(Uint32 & pid)
	{
		if (piece_providers.size() == 1)
		{
			pid = *piece_providers.begin();
			return true;
		}
		else
		{
			return false;
		}
	}

	void ChunkDownload::getStats(Stats & s)
	{
		s.chunk_index = chunk->getIndex();
		s.current_peer_id = getCurrentPeerID();
		s.download_speed = getDownloadSpeed();
		s.num_downloaders = getNumDownloaders();
		s.pieces_downloaded = num_downloaded;
		s.total_pieces = num;
	}
	
	bool ChunkDownload::isChoked() const
	{
		QPtrList<PeerDownloader>::const_iterator i = pdown.begin();
		while (i != pdown.end())
		{
			const PeerDownloader* pd = *i;
			// if there is one which isn't choked 
			if (!pd->isChoked())
				return false;
			i++;
		}
		return true;
	}
}
#include "chunkdownload.moc"
