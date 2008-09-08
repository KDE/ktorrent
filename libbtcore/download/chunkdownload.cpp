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
#include <diskio/piecedata.h>
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
		piece_data = new PieceData* [num]; // array of pointers to the piece data
		
		for (Uint32 i = 0;i < num;i++)
		{
			piece_data[i] = 0;
			piece_queue.append(i);
		}
		
		dstatus.setAutoDelete(true);

		num_pieces_in_hash = 0;
		hash_gen.start();
	}

	ChunkDownload::~ChunkDownload()
	{
		// make sure we do not keep pieces into memory unnecesary
		for (Uint32 i = 0;i < num;i++)
		{
			PieceData* piece = piece_data[i];
			if (piece)
			{
				piece->unref();
				chunk->savePiece(piece);
				piece_data[i] = 0;
			}
		}
		delete [] piece_data;
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
		
		PieceData* buf = chunk->getPiece(p.getOffset(),p.getLength(),false);
		if (buf)
		{
			buf->ref(); 
			piece_data[pp] = buf;
			ok = true;
			memcpy(buf->data(),p.getData(),p.getLength());	
			pieces.set(pp,true);
			piece_queue.removeAll(pp);
			piece_providers.insert(p.getPieceDownloader());
			num_downloaded++;
			if (pdown.count() > 1)
			{
				endgameCancel(p);
			}
			
			updateHash();
			
			if (num_downloaded >= num)
			{
				// finalize hash
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
		Q_UNUSED(reject);
		// find the peer 
		DownloadStatus* ds = dstatus.find(r.getPieceDownloader());
		if (ds)
		{
			//	Out(SYS_DIO|LOG_DEBUG) << "ds != 0"  << endl;
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
			
		int num_visited = 0;
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
		hdr.buffered = true; // unused 
		// save the chunk header
		file.write(&hdr,sizeof(ChunkDownloadHeader));
		// save the bitset
		file.write(pieces.getData(),pieces.getNumBytes());
		
		// save how many PieceHeader structs are to be written
		Uint32 num_pieces_to_follow = 0;
		for (Uint32 i = 0;i < hdr.num_bits;i++)
			if (piece_data[i])
				num_pieces_to_follow++;
		
		file.write(&num_pieces_to_follow,sizeof(Uint32));
			
		// save all buffered pieces
		for (Uint32 i = 0;i < hdr.num_bits;i++)
		{
			if (!piece_data[i])
				continue;
			
			PieceData* pd = piece_data[i];
			PieceHeader phdr;
			phdr.piece = i;
			phdr.size = pd->length();
			phdr.mapped = pd->mapped() ? 1 : 0;
			file.write(&phdr,sizeof(PieceHeader));
			if (!pd->mapped()) // buffered pieces need to be saved
			{
				file.write(pd->data(),pd->length());
			}
		}
	}
		
	bool ChunkDownload::load(File & file,ChunkDownloadHeader & hdr,bool update_hash)
	{
		// read pieces
		if (hdr.num_bits != num)
			return false; 
		
		pieces = BitSet(hdr.num_bits);
		file.read(pieces.getData(),pieces.getNumBytes());
		pieces.updateNumOnBits();
		
		num_downloaded = pieces.numOnBits();
		Uint32 num_pieces_to_follow = 0;
		if (file.read(&num_pieces_to_follow,sizeof(Uint32)) != sizeof(Uint32) || num_pieces_to_follow > num)
			return false;
		
		for (Uint32 i = 0;i < num_pieces_to_follow;i++)
		{
			PieceHeader phdr;
			if (file.read(&phdr,sizeof(PieceHeader)) != sizeof(PieceHeader))
				return false;
			
			if (phdr.piece >= num)
				return false;
			
			PieceData* p = chunk->getPiece(phdr.piece * MAX_PIECE_LEN,phdr.size,false);
			if (!p)
				return false;
			
			p->ref();
			if (!phdr.mapped)
			{
				if (file.read(p->data(),p->length()) != p->length())
				{
					p->unref();
					return false;
				}
			}
			piece_data[phdr.piece] = p;
		}
		
		for (Uint32 i = 0;i < pieces.getNumBits();i++)
			if (pieces.get(i))
				piece_queue.removeAll(i);
		
		// initialize hash
		if (update_hash)
		{
			Uint32 nn = 0;
			while (pieces.get(nn) && nn < num)
				nn++;
			
			for (Uint32 i = 0;i < nn;i++)
			{
				PieceData* piece = piece_data[i];
				Uint32 len = i == num - 1 ? last_size : MAX_PIECE_LEN;
				if (!piece)
					piece = chunk->getPiece(i*MAX_PIECE_LEN,len,true);
				
				if (!piece)
					return false;
				
				hash_gen.update(piece->data(),len);
			}
			
			num_pieces_in_hash = nn;
			
			updateHash();
		}
		
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
		while (nn < num && pieces.get(nn))
			nn++;
		
		for (Uint32 i = num_pieces_in_hash;i < nn;i++)
		{
			PieceData* piece = piece_data[i];
			Uint32 len = i == num - 1 ? last_size : MAX_PIECE_LEN;
			if (!piece)
				piece = chunk->getPiece(i*MAX_PIECE_LEN,len,true);
			
			if (piece)
			{
				hash_gen.update(piece->data(),len);
				// save the piece and set it to 0, we no longer need it
				piece->unref();
				chunk->savePiece(piece);
			}
			piece_data[i] = 0;
		}
		num_pieces_in_hash = nn;
	}
	
}
#include "chunkdownload.moc"
