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
#include "chunkdownload.h"
#include <klocale.h>
#include <algorithm>
#include <util/file.h>
#include <util/log.h>
#include <util/array.h>
#include <diskio/chunk.h>
#include <download/piece.h>
#include <interfaces/piecedownloader.h>

#include "downloader.h"

namespace bt
{	

	class DownloadStatus : public std::set<Uint32>
	{
	public:
	//	typedef std::set<Uint32>::iterator iterator;
		
		DownloadStatus()
		{
	
		}

		~DownloadStatus()
		{
		}

		void add(Uint32 p)
		{
			insert(p);
		}
		
		void remove(Uint32 p)
		{
			erase(p);
		}
		
		bool contains(Uint32 p)
		{
			return count(p) > 0;
		}
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

		num_pieces_in_hash = 0;
		if (usingContinuousHashing())
			hash_gen.start();

	}

	ChunkDownload::~ChunkDownload()
	{
		chunk->unref();
	}

	bool ChunkDownload::piece(const Piece & p,bool & ok)
	{
		ok = false;
		timer.update();
			
		Uint32 pp = p.getOffset() / MAX_PIECE_LEN;
		if (pieces.get(pp))
			return false;

	
		DownloadStatus* ds = dstatus.find(p.getPieceDownloader());
		if (ds)
			ds->remove(pp);
		
		Uint8* buf = chunk->getData();
		if (buf)
		{
			ok = true;
			memcpy(buf + p.getOffset(),p.getData(),p.getLength());	
			pieces.set(pp,true);
			piece_queue.removeAll(pp);
			piece_providers.insert(p.getPieceDownloader());
			num_downloaded++;
			if (pdown.count() > 1)
			{
				endgameCancel(p);
			}
			
			if (usingContinuousHashing())
				updateHash();
			
			if (num_downloaded >= num)
			{
				// finalize hash
				if (usingContinuousHashing())
					hash_gen.end();

				releaseAllPDs();
				return true;
			}
		}
		
		foreach (PieceDownloader* pd,pdown)
			sendRequests(pd);

		return false;
	}
	
	void ChunkDownload::releaseAllPDs()
	{
		foreach (PieceDownloader* pd,pdown)
		{
			pd->release();
			disconnect(pd,SIGNAL(timedout(const bt::Request& )),this,SLOT(onTimeout(const bt::Request& )));
			disconnect(pd,SIGNAL(rejected( const bt::Request& )),this,SLOT(onRejected( const bt::Request& )));
		}
		dstatus.clear();
		pdown.clear();
	}
	
	bool ChunkDownload::assign(PieceDownloader* pd)
	{
		if (!pd || pdown.contains(pd))
			return false;
			
		pd->grab();
		pdown.append(pd);
		dstatus.insert(pd,new DownloadStatus());
		sendRequests(pd);
		connect(pd,SIGNAL(timedout(const bt::Request& )),this,SLOT(onTimeout(const bt::Request& )));
		connect(pd,SIGNAL(rejected( const bt::Request& )),this,SLOT(onRejected( const bt::Request& )));
		return true;
	}
	
	void ChunkDownload::notDownloaded(const Request & r,bool reject)
	{
		// find the peer 
		DownloadStatus* ds = dstatus.find(r.getPieceDownloader());
		if (ds)
		{
			//	Out() << "ds != 0"  << endl;
			Uint32 p  = r.getOffset() / MAX_PIECE_LEN;
			ds->remove(p);
		}
			
			// go over all PD's and do requets again
		foreach (PieceDownloader* pd,pdown)
			sendRequests(pd);
	}
	
	void ChunkDownload::onRejected(const Request & r)
	{
		if (chunk->getIndex() == r.getIndex())
		{
			notDownloaded(r,true);
		}
	}
	
	void ChunkDownload::onTimeout(const Request & r)
	{
		// see if we are dealing with a piece of ours
		if (chunk->getIndex() == r.getIndex())
		{
			Out(SYS_CON|LOG_DEBUG) << QString("Request timed out %1 %2 %3 %4").arg(r.getIndex()).arg(r.getOffset()).arg(r.getLength()).arg(r.getPieceDownloader()->getName()) << endl;
		
			notDownloaded(r,false);
		}
	}
	
	void ChunkDownload::sendRequests(PieceDownloader* pd)
	{
		timer.update();
		DownloadStatus* ds = dstatus.find(pd);
		if (!ds)
			return;
			
		// if the peer is choked and we are not downloading an allowed fast chunk
		if (pd->isChoked())
			return;
			
		Uint32 num_visited = 0;
		while (num_visited < piece_queue.count() && pd->canAddRequest())
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
							i+1<num ? MAX_PIECE_LEN : last_size,
							pd));
				ds->add(i);
			}
			// move to the back so that it will take a while before it's turn is up
			piece_queue.pop_front();
			piece_queue.append(i);
			num_visited++;
		}
		
		if (piece_queue.count() < 2 && piece_queue.count() > 0)
			pd->setNearlyDone(true);
	}
	
	
	
	void ChunkDownload::update()
	{
		// go over all PD's and do requets again
		foreach (PieceDownloader* pd,pdown)
			sendRequests(pd);
	}
	
	
	void ChunkDownload::sendCancels(PieceDownloader* pd)
	{
		DownloadStatus* ds = dstatus.find(pd);
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
		QList<PieceDownloader*>::iterator i = pdown.begin();
		while (i != pdown.end())
		{
			PieceDownloader* pd = *i;
			DownloadStatus* ds = dstatus.find(pd);
			Uint32 pp = p.getOffset() / MAX_PIECE_LEN;
			if (ds && ds->contains(pp))
			{
				pd->cancel(Request(p));
				ds->remove(pp);
			}
			i++;
		}
	}

	void ChunkDownload::killed(PieceDownloader* pd)
	{
		if (!pdown.contains(pd))
			return;

		dstatus.erase(pd);
		pdown.removeAll(pd);
		disconnect(pd,SIGNAL(timedout(const bt::Request& )),this,SLOT(onTimeout(const bt::Request& )));
		disconnect(pd,SIGNAL(rejected( const bt::Request& )),this,SLOT(onRejected( const bt::Request& )));
	}
	
	Uint32 ChunkDownload::getChunkIndex() const
	{
		return chunk->getIndex();
	}

	QString ChunkDownload::getPieceDownloaderName() const
	{
		if (pdown.count() == 0)
		{
			return QString();
		}
		else if (pdown.count() == 1)
		{
			return pdown.first()->getName();
		}
		else
		{
			return i18n("%1 peers",pdown.count());
		}
	}

	Uint32 ChunkDownload::getDownloadSpeed() const
	{
		Uint32 r = 0;
		foreach (PieceDownloader* pd,pdown)
			r += pd->getDownloadRate();
		
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
		
	bool ChunkDownload::load(File & file,ChunkDownloadHeader & hdr)
	{
		// read pieces
		if (hdr.num_bits != num)
			return false; 
		
		pieces = BitSet(hdr.num_bits);
		Array<Uint8> data(pieces.getNumBytes());
		file.read(data,pieces.getNumBytes());
		pieces = BitSet(data,hdr.num_bits);
		num_downloaded = pieces.numOnBits();
		if (hdr.buffered)
		{
			// if it's a buffered chunk, load the data to
			if (file.read(chunk->getData(),chunk->getSize()) != chunk->getSize())
				return false;
		}
		
		for (Uint32 i = 0;i < pieces.getNumBits();i++)
			if (pieces.get(i))
				piece_queue.removeAll(i);
		
		updateHash();
		// add a 0 downloader, so that pieces downloaded
		// in a previous session cannot get a peer banned in this session
		if (num_downloaded) 
			piece_providers.insert(0);
		
		return true;
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
		QList<PieceDownloader*>::iterator i = pdown.begin();
		while (i != pdown.end())
		{
			sendCancels(*i);
			i++;
		}
	}

	PieceDownloader* ChunkDownload::getOnlyDownloader()
	{
		if (piece_providers.size() == 1)
		{
			return *piece_providers.begin();
		}
		else
		{
			return 0;
		}
	}

	void ChunkDownload::getStats(Stats & s)
	{
		s.chunk_index = chunk->getIndex();
		s.current_peer_id = getPieceDownloaderName();
		s.download_speed = getDownloadSpeed();
		s.num_downloaders = getNumDownloaders();
		s.pieces_downloaded = num_downloaded;
		s.total_pieces = num;
	}
	
	bool ChunkDownload::isChoked() const
	{
		QList<PieceDownloader*>::const_iterator i = pdown.begin();
		while (i != pdown.end())
		{
			const PieceDownloader* pd = *i;
			// if there is one which isn't choked 
			if (!pd->isChoked())
				return false;
			i++;
		}
		return true;
	}
	
	void ChunkDownload::updateHash()
	{
		// update the hash until where we can
		Uint32 nn = num_pieces_in_hash;
		while (pieces.get(nn) && nn < num)
			nn++;
		
		for (Uint32 i = num_pieces_in_hash;i < nn;i++)
		{
			const Uint8* data = chunk->getData() + i * MAX_PIECE_LEN;
			hash_gen.update(data,i == num - 1 ? last_size : MAX_PIECE_LEN);
		}
		num_pieces_in_hash = nn;
	}
	
	bool ChunkDownload::usingContinuousHashing() const
	{
		// if the pieces are larger then 1 MB we will be using the continuous hashing feature
		return pieces.getNumBits() > 64;
	}
}
#include "chunkdownload.moc"
