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
#include "choker.h"
#include "peermanager.h"
#include "peer.h"
#include "packetwriter.h"

namespace bt
{

	Choker::Choker(PeerManager & pman) : pman(pman)
	{
		opt_unchoke_index = 0;
		opt_unchoke = 1;
	}


	Choker::~Choker()
	{}

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

	void Choker::updateInterested()
	{
		for (Uint32 i = 0;i < pman.getNumConnectedPeers();i++)
		{
			Peer* p = pman.getPeer(i);
			if (p->isInterested())
			{
				interested.push_back(p);
			}
			else
			{
				not_interested.push_back(p);
			}
		}
	}

	void Choker::updateDownloaders()
	{
		std::list<Peer*>::iterator itr = interested.begin();
		int num = 0;
		// send all downloaders an unchoke
		for (;itr != interested.end();itr++)
		{
			Peer* p = *itr;
			if (num < 4)
			{
				p->getPacketWriter().sendUnchoke();
				downloaders.push_back(p);
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
		if (downloaders.size() == 0)
			return;
		
		std::list<Peer*>::iterator itr = not_interested.begin();
		// fd = fastest_downloader
		Peer* fd = downloaders.front();
		// send all downloaders an unchoke
		for (;itr != not_interested.end();itr++)
		{
			Peer* p = *itr;
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
		if (opt_unchoke == 3)
		{
			Peer* p = pman.getPeer(opt_unchoke_index);
			if (p)
			{
				PacketWriter & pout = p->getPacketWriter();
				pout.sendUnchoke();
				opt_unchoke_index = (opt_unchoke_index + 1) % pman.getNumConnectedPeers();
				opt_unchoke = 1;
			}
			else
			{
				opt_unchoke_index = 0;
			}
		}
		else
		{
			opt_unchoke++;
		}
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
			interested.sort(DownloadRateCmp());
			not_interested.sort(DownloadRateCmp());
		}
		else
		{
			interested.sort(UploadRateCmp());
			not_interested.sort(UploadRateCmp());
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
