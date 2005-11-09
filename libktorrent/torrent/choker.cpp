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
#include <qptrlist.h>
#include <interfaces/functions.h>
#include "choker.h"
#include "peermanager.h"
#include "peer.h"
#include "packetwriter.h"

using namespace kt;

namespace bt
{
	
	PeerPtrList::PeerPtrList(bool dl_cmp) : dl_cmp(dl_cmp)
	{}
		
	PeerPtrList::~PeerPtrList()
	{}
		
	int PeerPtrList::compareItems(QPtrCollection::Item a, QPtrCollection::Item b)
	{
		Peer* pa = (Peer*)a;
		Peer* pb = (Peer*)b;
		if (dl_cmp)
			return CompareVal(pa->getDownloadRate(),pb->getDownloadRate());
		else
			return CompareVal(pa->getUploadRate(),pb->getUploadRate());
	}

	Choker::Choker(PeerManager & pman) : pman(pman)
	{
		opt_unchoke_index = 0;
		opt_unchoke = 1;
	}


	Choker::~Choker()
	{}
#if 0
	struct UploadRateCmp
	{
		bool operator () (Peer* a,Peer* b)
		{
			return a->getUploadRate() > b->getUploadRate();
		}
	};
	
	struct DownloadRateCmp
	{
		bool operator () (Peer* a,Peer* b)
		{
			return a->getDownloadRate() > b->getDownloadRate();
		}
	};
#endif
	void Choker::updateInterested()
	{
		for (Uint32 i = 0;i < pman.getNumConnectedPeers();i++)
		{
			Peer* p = pman.getPeer(i);
			
			if (p->getID() == opt_unchoked_peer_id)
				continue;
			
			if (p->isInterested())
			{
				interested.append(p);
			}
			else
			{
				not_interested.append(p);
			}
		}
	}

	void Choker::updateDownloaders()
	{
		QPtrList<Peer>::iterator itr = interested.begin();
		int num = 0;
		// send all downloaders an unchoke
		for (;itr != interested.end();itr++)
		{
			Peer* p = *itr;
			
			if (p->getID() == opt_unchoked_peer_id)
				continue;
			
			if (num < 4)
			{
				p->getPacketWriter().sendUnchoke();
				downloaders.append(p);
				num++;
			}
			else
			{
				p->getPacketWriter().sendChoke();
			}
		}
	}

	void Choker::sendInterested(bool have_all)
	{
		for (Uint32 i = 0;i < pman.getNumConnectedPeers();i++)
		{
			Peer* p = pman.getPeer(i);
			PacketWriter & pout = p->getPacketWriter();
			// if we don't have the entire file, send an intereseted message,
			// else we're not intereseted
			if (have_all && p->areWeInterested())
				pout.sendNotInterested();
			else if (!have_all && !p->areWeInterested())
				pout.sendInterested();
		}
	}

	void Choker::sendUnchokes(bool have_all)
	{
		if (downloaders.count() == 0)
			return;
		
		QPtrList<Peer>::iterator itr = not_interested.begin();
		// fd = fastest_downloader
		Peer* fd = downloaders.first();
		// send all downloaders an unchoke
		for (;itr != not_interested.end();itr++)
		{
			Peer* p = *itr;
			if (p->getID() == opt_unchoked_peer_id)
				continue;
			
			if ((have_all && p->getDownloadRate() > fd->getDownloadRate()) ||
				(!have_all && p->getUploadRate() > fd->getUploadRate()))
			{
				p->getPacketWriter().sendUnchoke();
			}
			else
			{
				p->getPacketWriter().sendChoke();
			}
		}
	}

	void Choker::optimisticUnchoke()
	{
		if (pman.getNumConnectedPeers() == 0)
			return;

		// only switch optimistic unchoked peer every 30 seconds
		// (update interval of choker is 10 seconds)
		if (opt_unchoke != 3)
		{
			opt_unchoke++;
			return;
		}
		
		// Get current time
		QTime now = QTime::currentTime();
		QPtrList<Peer> peers;	// list to store peers to select from

		// recently connected peers == peers connected in the last 5 minutes
		const int RECENTLY_CONNECT_THRESH = 5*60;
		
		for (Uint32 i = 0;i < pman.getNumConnectedPeers();i++)
		{
			Peer* p = pman.getPeer(i);
			if (p->getConnectTime().secsTo(now) < RECENTLY_CONNECT_THRESH)
			{
				// we favor recently connected peers 3 times over other peers
				// so we add them 3 times to the list
				peers.append(p);
				peers.append(p);
				peers.append(p);
			}
			else
			{
				// not recent, so just add one time
				peers.append(p);
			}
		}

		// draw a random one from the list and send it an unchoke
		opt_unchoke_index = rand() % peers.count();
		Peer* lucky_one = peers.at(opt_unchoke_index);
		lucky_one->getPacketWriter().sendUnchoke();
		opt_unchoked_peer_id = lucky_one->getID();
		opt_unchoke = 1;
	}

	void Choker::update(bool have_all)
	{
		if (pman.getNumConnectedPeers() == 0)
			return;
		
		downloaders.clear();
		interested.clear();
		not_interested.clear();

		// first alert everybody that we're interested or not
		sendInterested(have_all);
		// get who is interested and not
		updateInterested();
		// them sort them;
		if (have_all)
		{
			interested.setDLCmp(true);
			interested.sort();
			not_interested.setDLCmp(true);
			not_interested.sort();
		}
		else
		{
			interested.setDLCmp(false);
			interested.sort();
			not_interested.setDLCmp(false);
			not_interested.sort();
		}
		// determine the downloaders
		updateDownloaders();
		// unchoke the not_interested peers
		// which have a faster upload rate then the downloaders
		sendUnchokes(have_all);
		// optimisticly unchoke somebody
		optimisticUnchoke();
	}
	
}
