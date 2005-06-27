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

#include <algorithm>
#include "file.h"
#include "log.h"
#include "chunkdownload.h"
#include "downloader.h"
#include "chunk.h"
#include "peer.h"
#include "peermanager.h"
#include "piece.h"
#include "peerdownloader.h"

namespace bt
{
	const int PIECE_NOT_DOWNLOADED = 0;
	const int PIECE_REQUESTED = 1;
	const int PIECE_DOWNLOADED = 2;

	class DownloadStatus
	{
		int* status;
		Uint32 num;
	public:
		DownloadStatus(bool* s,Uint32 num) : num(num)
		{
			status = new int[num];
			for (Uint32 i = 0;i < num;i++)
				status[i] = s[i] ? PIECE_DOWNLOADED : PIECE_NOT_DOWNLOADED;
		}

		~DownloadStatus()
		{
			delete [] status;
		}

		int get(Uint32 p) const
		{
			return status[p];
		}
		
		void set(Uint32 p,int s)
		{
			status[p] = s;
		}
		
		void fill(int s)
		{
			std::fill(status,status+num,s);
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
		
		buf = new Uint8[chunk->getSize()];
		pieces = new bool[num];
		std::fill(pieces,pieces+num,false),
		
		dstatus.setAutoDelete(true);
	}

	ChunkDownload::~ChunkDownload()
	{
		delete [] buf;
		delete [] pieces;
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
		if (pieces[pp])
		{
		//	Out() << "pieces[pp] == PIECE_DOWNLOADED" << endl;
			return false;
		}

	//	Out() << "Piece " << p.getIndex() << " " << p.getOffset() << " " << pp << endl;
		const Peer* peer = p.getPeer();
		DownloadStatus* ds = dstatus.find(peer);
		memcpy(buf + p.getOffset(),p.getData(),p.getLength());
		ds->set(pp,PIECE_DOWNLOADED);
		pieces[pp] = true;
		num_downloaded++;
		
		if (pdown.count() > 1)
		{
			endgameCancel(p);
		}

		if (num_downloaded == num)
		{
			chunk->setData(buf);
			buf = 0;
			if (pdown.count() == 1)
			{
				pdown.at(0)->release();
				pdown.clear();
			}
			return true;
		}
		
		for (QPtrList<PeerDownloader>::iterator i = pdown.begin();i != pdown.end();i++)
			sendRequests(*i);

		return false;
	}
	
	void ChunkDownload::assignPeer(PeerDownloader* pd,bool endgame)
	{
		if (!pd)
			return;
		
		if (!endgame && pdown.count() == 1)
		{
			sendCancels(pdown.at(0));
			pdown.clear();
		}
		
		if (pdown.contains(pd))
			return;
		

		pdown.append(pd);
		const Peer* peer = pd->getPeer();
		dstatus.insert(peer,new DownloadStatus(pieces,num));
		sendRequests(pd);
	}
	
	void ChunkDownload::sendRequests(PeerDownloader* pd)
	{
		timer.update();
		DownloadStatus* ds = dstatus.find(pd->getPeer());
		for (Uint32 i = 0;i < num && pd->getNumRequests() < 8;i++)
		{
			if (ds->get(i) == PIECE_NOT_DOWNLOADED ||
				(num - num_downloaded == 1 && ds->get(i) == PIECE_REQUESTED))
			{
				pd->download(
						Request(
							chunk->getIndex(),
							i*MAX_PIECE_LEN,
							i+1<num ? MAX_PIECE_LEN : last_size,0));
				ds->set(i,PIECE_REQUESTED);
			}
		}
	}
	
	
	void ChunkDownload::sendCancels(PeerDownloader* pd)
	{
		DownloadStatus* ds = dstatus.find(pd->getPeer());
		for (Uint32 i = 0;i < num;i++)
		{
			if (ds->get(i) == PIECE_REQUESTED)
			{
				pd->cancel(
						Request(
							chunk->getIndex(),
							i*MAX_PIECE_LEN,
							i+1<num ? MAX_PIECE_LEN : last_size,0));
				ds->set(i,PIECE_NOT_DOWNLOADED);
			}
			
		}
		timer.update();
	}
	
	void ChunkDownload::endgameCancel(const Piece & p)
	{
		QPtrList<PeerDownloader>::iterator i = pdown.begin();
		while (i != pdown.end())
		{
			PeerDownloader* pd = *i;
			DownloadStatus* ds = dstatus.find(pd->getPeer());
			if (ds->get(p.getIndex()) == PIECE_REQUESTED)
			{
				pd->cancel(Request(p));
				ds->set(p.getIndex(),PIECE_DOWNLOADED);
			}
			i++;
		}
	}

	void ChunkDownload::peerKilled(PeerDownloader* pd)
	{
		if (!pdown.contains(pd))
			return;

		dstatus.erase(pd->getPeer());
		pdown.remove(pd);
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
			return p->getPeerID().toString();
		}
		else
		{
			return QString("%1 peers").arg(pdown.count());
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
		file.write(pieces,sizeof(bool)*num);
		file.write(buf,chunk->getSize());
	}
		
	void ChunkDownload::load(File & file)
	{
		file.read(pieces,sizeof(bool)*num);
		file.read(buf,chunk->getSize());
		num_downloaded = 0;
		for (Uint32 i = 0;i < num;i++)
		{
			if (pieces[i])
				num_downloaded++;
		}
	}

	Uint32 ChunkDownload::bytesDownloaded() const
	{
		Uint32 num_bytes = 0;
		for (Uint32 i = 0;i < num;i++)
		{
			if (pieces[i])
			{
				num_bytes += i == num-1 ? last_size : MAX_PIECE_LEN;
			}
		}
		return num_bytes;
	}
}
#include "chunkdownload.moc"
