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

	PeerDownloader::PeerDownloader(Peer* peer) : peer(peer),grabbed(0)
	{
		connect(peer,SIGNAL(piece(const Piece& )),this,SLOT(piece(const Piece& )));
		connect(peer,SIGNAL(destroyed()),this,SLOT(peerDestroyed()));
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

	Uint32 PeerDownloader::getNumRequests() const 
	{
		return reqs.count() + unsent_reqs.count();
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
		
		TimeStampedRequest r = TimeStampedRequest(req);
		reqs.append(r);
		peer->getPacketWriter().sendRequest(req);
	}
	
	void PeerDownloader::cancel(const Request & req)
	{
		if (!peer)
			return;

		if (reqs.contains(req))
		{
			reqs.remove(req);
			peer->getPacketWriter().sendCancel(req);
		}
		else
		{
			unsent_reqs.remove(req);
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
	
		reqs.clear();
		unsent_reqs.clear();
	}
		
	void PeerDownloader::piece(const Piece & p)
	{
		Request r(p);
		if (reqs.contains(r))
		{
			reqs.remove(r);
			downloaded(p);
		}
		else 
		{
			// this is probably a timed out request
			// emit the signal and let the chunkdownload handle it
			downloaded(p);
		}
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

	
	void PeerDownloader::downloadUnsent()
	{
		if (!peer)
			return;

		QValueList<TimeStampedRequest>::iterator i = unsent_reqs.begin();
		while (i != unsent_reqs.end())
		{
			TimeStampedRequest & tr = *i;
			// update time stamp
			tr.time_stamp = bt::GetCurrentTime();
			reqs.append(tr);
			peer->getPacketWriter().sendRequest(tr.req);
			i = unsent_reqs.erase(i);
		}
	}

	void PeerDownloader::downloadOneUnsent()
	{
		if (unsent_reqs.empty())
			return;

		TimeStampedRequest & tr = unsent_reqs.first();
		tr.time_stamp = bt::GetCurrentTime();
		reqs.append(tr);
		peer->getPacketWriter().sendRequest(tr.req);
		unsent_reqs.pop_front();
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
				timedout(tr.req);
				// cancel it
				peer->getPacketWriter().sendCancel(tr.req);
				i = reqs.erase(i);
			}
			else
			{ 
				i++;
			}
		}
	}
	
	Uint32 PeerDownloader::getMaximumOutstandingReqs() const
	{
		// get the download rate in KB/sec
		double rate_kbs = (double)peer->getDownloadRate() / 1024.0;
		// per 15 KB/s we can assign one downloader
		Uint32 num_extra = (Uint32)floor(rate_kbs / 1);
		
		// we have a maximum of 25 outstanding chunk requests
		if (num_extra > max_outstanding_reqs)
			num_extra = max_outstanding_reqs;
		
		return 10 + num_extra;
	}

	Uint32 PeerDownloader::getMaxChunkDownloads() const
	{
		// get the download rate in KB/sec
		double rate_kbs = (double)peer->getDownloadRate() / 1024.0;
		// per 15 KB/s we can assign one downloader
		Uint32 num_extra = (Uint32)floor(rate_kbs / 15.0);
		// if we nearly finished the last chunk increment num_extra
		if (getMaximumOutstandingReqs() <= 2)
			num_extra++;
		// lets not overreact
		if (num_extra > 5)
			num_extra = 5;
					
		return 1 + num_extra;
	}
	
	Uint32 PeerDownloader::max_outstanding_reqs = 20;
	
	void PeerDownloader::setMemoryUsage(Uint32 m)
	{
		switch (m)
		{
			case 2:
				max_outstanding_reqs = 40;
				break;
			case 1:
				max_outstanding_reqs = 30;
				break;
			case 0:
			default:
				max_outstanding_reqs = 20;
				break;
		}
	}
	
}
#include "peerdownloader.moc"
