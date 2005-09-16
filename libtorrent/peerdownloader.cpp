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
#include <libutil/functions.h>
#include "peerdownloader.h"
#include "peer.h"
#include "piece.h"
#include "packetwriter.h"
#include "downloadcap.h"


namespace bt
{

	PeerDownloader::PeerDownloader(Peer* peer) : peer(peer),grabbed(0)
	{
		connect(peer,SIGNAL(piece(const Piece& )),this,SLOT(piece(const Piece& )));
		connect(peer,SIGNAL(destroyed()),this,SLOT(peerDestroyed()));

		last_req_time = 0;
		req_time_interval = 0;
		blocked = false;

		DownloadCap::instance().addPeerDonwloader(this);
	}


	PeerDownloader::~PeerDownloader()
	{
		DownloadCap::instance().removePeerDownloader(this);
	}

	void PeerDownloader::setRequestInterval(Uint32 rti)
	{
		this->req_time_interval = rti;
	}

	void PeerDownloader::retransmitRequests()
	{
		for (QValueList<Request>::iterator i = reqs.begin();i != reqs.end();i++)
			peer->getPacketWriter().sendRequest(*i);
			
	}

	Uint32 PeerDownloader::getNumRequests() const 
	{
		return reqs.count();
	}
	
	int PeerDownloader::grab()
	{
		if (peer->isChoked() || grabbed >= 2)
			return 0;
		
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

		Uint32 now = bt::GetCurrentTime();
		if (now - last_req_time >= req_time_interval && !blocked)
		{
			reqs.append(req);
			peer->getPacketWriter().sendRequest(req);
			last_req_time = now;
		}
		else
		{
			unsent_reqs.append(req);
		}
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
			QValueList<Request>::iterator i = reqs.begin();
			while (i != reqs.end())
			{
				peer->getPacketWriter().sendCancel(*i);
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
		if (!peer || blocked)
			return;

		QValueList<Request>::iterator i = unsent_reqs.begin();
		while (i != unsent_reqs.end())
		{
			Uint32 now = bt::GetCurrentTime();
			if (now - last_req_time >= req_time_interval)
			{
				reqs.append(*i);
				peer->getPacketWriter().sendRequest(*i);
				i = unsent_reqs.erase(i);
				last_req_time = now;
			}
			else
			{
				i++;
				return;
			}
		}
	}

	Uint32 PeerDownloader::getDownloadRate() const
	{
		if (peer)
			return peer->getDownloadRate();
		else
			return 0;
	}
	
}
#include "peerdownloader.moc"
