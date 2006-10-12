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
		return wait_queue.count() < 50;
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
		// we use a 60 second interval
		const Uint32 MAX_INTERVAL = 60 * 1000;
		QValueList<TimeStampedRequest>::iterator i = reqs.begin();
		while (i != reqs.end())
		{
			TimeStampedRequest & tr = *i;
			if (bt::GetCurrentTime() - tr.time_stamp > MAX_INTERVAL)
			{
			//	Out() << "Request " << tr.req.getIndex() << " " << tr.req.getOffset() << " timed out !" << endl;
				// cancel it
				Request r = tr.req;
				peer->getPacketWriter().sendCancel(tr.req);
				i = reqs.erase(i);
				timedout(r);
				// we now have a timeout
				if (!peer->isChoked() && peer->isSnubbed())
					peer->stats.evil = true;
			}
			else
			{ 
				// new requests get appended so once we have found one
				// which hasn't timed out all the following will also not have timed out
				break;
			}
		}
	}
	
	void PeerDownloader::addAllowedFastChunk(Uint32 chunk)
	{
		allowed_fast.insert(chunk);
	}
	
	bool PeerDownloader::inAllowedFastChunks(Uint32 chunk)  const
	{
		return allowed_fast.count(chunk) > 0;
	}
	
	/*
	Uint32 PeerDownloader::getMaximumOutstandingReqs() const
	{
		// get the download rate in KB/sec
		double pieces_per_sec = (double)peer->getDownloadRate() / MAX_PIECE_LEN;
		
		if (pieces_per_sec < 1.0)
			return 5;
		else
			return 5 + (Uint32)ceil(2*pieces_per_sec);
	}
	*/

	Uint32 PeerDownloader::getMaxChunkDownloads() const
	{
		// get the download rate in KB/sec
		Uint32 rate_kbs = peer->getDownloadRate() / 1024;
		Uint32 num_extra = 0;
		if (rate_kbs >= 50)
			num_extra = 1;
		else if (rate_kbs >= 100)
			num_extra = 2;
		else if (rate_kbs >= 150)
			num_extra = 3;
		
		// also take into account the size of each chunk
		// if a chunk has less then 16 pieces we multiply by 16 / num_pieces_per_chunk
		Uint32 mul_factor = 1;
		
		if (chunk_size >= 16)
			mul_factor = 1;
		else
			mul_factor = 16 / chunk_size;
		 
		return mul_factor * (1 + num_extra);
	}
	
	void PeerDownloader::choked()
	{
		QValueList<TimeStampedRequest>::iterator i = reqs.begin();
		while (i != reqs.end())
		{
			TimeStampedRequest & tr = *i;
			if (allowed_fast.count(tr.req.getIndex()) == 0)
			{
				rejected(tr.req);
				i = reqs.erase(i);
			}
			else
				i++;
		}
		
		QValueList<Request>::iterator j = wait_queue.begin();
		while (j != wait_queue.end())
		{
			Request & req = *j;
			if (allowed_fast.count(req.getIndex()) == 0)
			{
				rejected(req);
				j = wait_queue.erase(j);
			}
			else
				j++;
		}
	}
	
	void PeerDownloader::update()
	{
		// modify the interval if necessary
		double pieces_per_sec = (double)peer->getDownloadRate() / MAX_PIECE_LEN;
		
		Uint32 max_reqs = 1 + (Uint32)ceil(pieces_per_sec);
		
		while (wait_queue.count() > 0 && reqs.count() < max_reqs)
		{
			// get a request from the wait queue and send that
			Request req = wait_queue.front();
			wait_queue.pop_front();
			TimeStampedRequest r = TimeStampedRequest(req);
			reqs.append(r);
			peer->getPacketWriter().sendRequest(req);
		}
	}
}

#include "peerdownloader.moc"
