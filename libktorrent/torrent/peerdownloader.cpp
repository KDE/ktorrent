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
#include <math.h>
#include <util/functions.h>
#include <util/log.h>
#include "globals.h"
#include "peerdownloader.h"
#include "peer.h"
#include "piece.h"
#include "packetwriter.h"


namespace bt
{
	TimeStampedRequest::TimeStampedRequest()
	{
		time_stamp = bt::GetCurrentTime();
	}
			
	TimeStampedRequest::TimeStampedRequest(const Request & r) : req(r)
	{
		time_stamp = bt::GetCurrentTime();
	}
	
	TimeStampedRequest::TimeStampedRequest(const TimeStampedRequest & t) 
		: req(t.req),time_stamp(t.time_stamp)
	{
	}
		
	bool TimeStampedRequest::operator == (const Request & r)
	{
		return r == req;
	}
	
	bool TimeStampedRequest::operator == (const TimeStampedRequest & r)
	{
		return r.req == req;
	}
	
	TimeStampedRequest & TimeStampedRequest::operator = (const Request & r)
	{
		time_stamp = bt::GetCurrentTime();
		req = r;
		return *this;
	}
	
	TimeStampedRequest & TimeStampedRequest::operator = (const TimeStampedRequest & r)
	{
		time_stamp = r.time_stamp;
		req = r.req;
		return *this;
	}

	PeerDownloader::PeerDownloader(Peer* peer,Uint32 chunk_size) : peer(peer),grabbed(0),chunk_size(chunk_size / MAX_PIECE_LEN)
	{
		connect(peer,SIGNAL(piece(const Piece& )),this,SLOT(piece(const Piece& )));
		connect(peer,SIGNAL(destroyed()),this,SLOT(peerDestroyed()));
		nearly_done = false;
		max_wait_queue_size = 25;
	}


	PeerDownloader::~PeerDownloader() 
	{
	}
#if 0
	void PeerDownloader::retransmitRequests()
	{
		for (QValueList<Request>::iterator i = reqs.begin();i != reqs.end();i++)
			peer->getPacketWriter().sendRequest(*i);
			
	}
#endif

	bool PeerDownloader::canAddRequest() const
	{
		return wait_queue.count() < max_wait_queue_size;
	}

	Uint32 PeerDownloader::getNumRequests() const 
	{
		return reqs.count() /*+ wait_queue.count() */;
	}
	
	int PeerDownloader::grab()
	{
		grabbed++;
		return grabbed;
	}
	
	void PeerDownloader::release()
	{
		grabbed--;
		if (grabbed < 0)
			grabbed = 0;
	}

	void PeerDownloader::download(const Request & req)
	{
		if (!peer)
			return;
		
		wait_queue.append(req);
		update();
	}
	
	void PeerDownloader::cancel(const Request & req)
	{
		if (!peer)
			return;
		
		if (wait_queue.contains(req))
		{
			wait_queue.remove(req);
		}
		else if (reqs.contains(req))
		{
			reqs.remove(req);
			peer->getPacketWriter().sendCancel(req);
		}
	}
	
	void PeerDownloader::onRejected(const Request & req)
	{
		if (!peer)
			return;

//		Out(SYS_CON|LOG_DEBUG) << "Rejected : " << req.getIndex() << " " 
//				<< req.getOffset() << " " << req.getLength() << endl;
		if (reqs.contains(req))
		{
			reqs.remove(req);
			rejected(req);
		}
	}
	
	void PeerDownloader::cancelAll()
	{
		if (peer)
		{
			QValueList<TimeStampedRequest>::iterator i = reqs.begin();
			while (i != reqs.end())
			{
				TimeStampedRequest & tr = *i;
				peer->getPacketWriter().sendCancel(tr.req);
				i++;
			}
		}
	
		wait_queue.clear();
		reqs.clear();
	}
		
	void PeerDownloader::piece(const Piece & p)
	{
		Request r(p);
		if (wait_queue.contains(r))
			wait_queue.remove(r);
		else if (reqs.contains(r))
			reqs.remove(r);
			
		downloaded(p);
		update();
	}
	
	void PeerDownloader::peerDestroyed()
	{
		peer = 0;
	}
	
	bool PeerDownloader::isChoked() const
	{
		if (peer)
			return peer->isChoked();
		else
			return true;
	}
	
	bool PeerDownloader::hasChunk(Uint32 idx) const
	{
		if (peer)
			return peer->getBitSet().get(idx);
		else
			return false;
	}

	Uint32 PeerDownloader::getDownloadRate() const
	{
		if (peer)
			return peer->getDownloadRate();
		else
			return 0;
	}
	
	void PeerDownloader::checkTimeouts()
	{
		TimeStamp now = bt::GetCurrentTime(); 
		// we use a 60 second interval
		const Uint32 MAX_INTERVAL = 60 * 1000;
		QValueList<TimeStampedRequest>::iterator i = reqs.begin();
		while (i != reqs.end())
		{
			TimeStampedRequest & tr = *i;
			if (now - tr.time_stamp > MAX_INTERVAL)
			{
				// cancel it
				TimeStampedRequest r = tr;
				peer->getPacketWriter().sendCancel(r.req);
				
				// retransmit it
				peer->getPacketWriter().sendRequest(r.req);
				r.time_stamp = now;
				
				// reappend it at the end of the list
				i = reqs.erase(i);
				reqs.append(r);
				Out(SYS_CON|LOG_DEBUG) << "Retransmitting " << r.req.getIndex() << ":" << r.req.getOffset() << endl;
			}
			else
			{ 
				// new requests get appended so once we have found one
				// which hasn't timed out all the following will also not have timed out
				break;
			}
		}
	}
	
	
	Uint32 PeerDownloader::getMaxChunkDownloads() const
	{
		// get the download rate in KB/sec
		Uint32 rate_kbs = peer->getDownloadRate();
		rate_kbs = rate_kbs / 1024;
		Uint32 num_extra = rate_kbs / 50;
		
		if (chunk_size >= 16)
		{
			return 1 + 16 * num_extra / chunk_size;
		}
		else
		{
			return 1 + (16 / chunk_size) * num_extra;
		}
	}
	
	void PeerDownloader::choked()
	{
		// choke doesn't mean reject when fast extensions are enabled
		if (peer->getStats().fast_extensions)
			return;
		
		QValueList<TimeStampedRequest>::iterator i = reqs.begin();
		while (i != reqs.end())
		{
			TimeStampedRequest & tr = *i;
			rejected(tr.req);
			i++;
		}
		reqs.clear();
		
		QValueList<Request>::iterator j = wait_queue.begin();
		while (j != wait_queue.end())
		{
			Request & req = *j;
			rejected(req);
			j++;
		}
		wait_queue.clear();
	}
	
	void PeerDownloader::update()
	{
		// modify the interval if necessary
		double pieces_per_sec = (double)peer->getDownloadRate() / MAX_PIECE_LEN;
		
		Uint32 max_reqs = 1 + (Uint32)ceil(10*pieces_per_sec);
		
		while (wait_queue.count() > 0 && reqs.count() < max_reqs)
		{
			// get a request from the wait queue and send that
			Request req = wait_queue.front();
			wait_queue.pop_front();
			TimeStampedRequest r = TimeStampedRequest(req);
			reqs.append(r);
			peer->getPacketWriter().sendRequest(req);
		}
		
		max_wait_queue_size = 2*max_reqs;
		if (max_wait_queue_size < 10)
			max_wait_queue_size = 10;
	}
}

#include "peerdownloader.moc"
