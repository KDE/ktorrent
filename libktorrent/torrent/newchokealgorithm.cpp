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
#include <util/functions.h>
#include <interfaces/functions.h>
#include "newchokealgorithm.h"
#include "peermanager.h"
#include "peer.h"
#include "packetwriter.h"
#include "peeruploader.h"

using namespace kt;

namespace bt
{
	const Uint32 UNDEFINED_ID = 0xFFFFFFFF;
	

	NewChokeAlgorithm::NewChokeAlgorithm(): ChokeAlgorithm()
	{
		round_state = 1;
	}


	NewChokeAlgorithm::~NewChokeAlgorithm()
	{}


	void NewChokeAlgorithm::doChoking(PeerManager& pman, bool have_all)
	{
		// first update the master list
	//	fillMaster(pman);
		if (have_all)
			doChokingSeederState(pman);
		else
			doChokingLeecherState(pman);
	}
	
	int RevDownloadRateCmp(Peer* a,Peer* b)
	{
		if (b->getDownloadRate() > a->getDownloadRate())
			return 1;
		else if (a->getDownloadRate() > b->getDownloadRate())
			return -1;
		else
			return 0;
	}
	
	void NewChokeAlgorithm::doChokingLeecherState(PeerManager& pman)
	{
		Uint32 num_peers = pman.getNumConnectedPeers();
		if (num_peers == 0)
			return;
		
		Uint32 now = GetCurrentTime();
		Peer* poup = pman.findPeer(opt_unchoked_peer_id);
		Peer* unchokers[] = {0,0,0,0};
		
		// first find the planned optimistic unchoked peer if we are in the correct round
		if (round_state == 1 || poup == 0)
		{
			opt_unchoked_peer_id = findPlannedOptimisticUnchokedPeer(pman);
			poup = pman.findPeer(opt_unchoked_peer_id);
		}
		
		PeerPtrList peers,other;
		// now get all the peers who are interested and have sent us a piece in the
		// last 30 seconds
		for (Uint32 i = 0;i < num_peers;i++)
		{
			Peer* p = pman.getPeer(i);
			if (!p)
				continue;
			
			if (p->isInterested() && now - p->getTimeSinceLastPiece() <= 30000)
				peers.append(p);
			else
				other.append(p);
		}
		
		// sort them using a reverse download rate compare
		// so that the fastest downloaders are in front
		peers.setCompareFunc(RevDownloadRateCmp);
		peers.sort();
		other.setCompareFunc(RevDownloadRateCmp);
		other.sort();
		
		
		// get the first tree and punt them in the unchokers
		for (Uint32 i = 0;i < 3;i++)
		{
			if (i < peers.count())
			{
				unchokers[i] = peers.at(i);
			}
		}
		
		// see if poup if part of the first 3
		// and if necessary replace it
		bool poup_in_unchokers = false;
		Uint32 attempts = 0;
		do 
		{
			poup_in_unchokers = false;
			for (Uint32 i = 0;i < 3;i++)
			{
				if (unchokers[i] != poup)
					continue;
				
				opt_unchoked_peer_id = findPlannedOptimisticUnchokedPeer(pman);
				poup = pman.findPeer(opt_unchoked_peer_id);
				poup_in_unchokers = true;
				break;
			}	
			// we don't want to keep trying this forever, so limit it to 5 atttempts
			attempts++;
		}while (poup_in_unchokers && attempts < 5);
		
		unchokers[4] = poup;
		
		Uint32 other_idx = 0;
		Uint32 peers_idx = 3;
		// unchoke the 4 unchokers 
		for (Uint32 i = 0;i < 4;i++)
		{
			if (!unchokers[i])
			{
				// pick some other peer to unchoke
				unchokers[i] = peers.at(peers_idx++);
				if (unchokers[i] == poup) // it must not be equal to the poup
					unchokers[i] = peers.at(peers_idx++);
				
				// nobody in the peers list, try the others list
				if (!unchokers[i])
					unchokers[i] = other.at(other_idx++);
			}
			
			if (unchokers[i])
				unchokers[i]->getPacketWriter().sendUnchoke();
		}
		
		// choke the rest
		for (Uint32 i = 0;i < num_peers;i++)
		{
			Peer* p = pman.getPeer(i);
			if (p == unchokers[0] || p == unchokers[1] || p == unchokers[2] || p == unchokers[3])
				continue;
			if (p)
				p->getPacketWriter().sendChoke();
		}
		
		round_state++;
		if (round_state > 3)
			round_state = 1;
	}
	
	Uint32 NewChokeAlgorithm::findPlannedOptimisticUnchokedPeer(PeerManager& pman)
	{
		Uint32 num_peers = pman.getNumConnectedPeers();
		if (num_peers == 0)
			return UNDEFINED_ID;
			
		// find a random peer that is choked and interested
		Uint32 start = rand() % num_peers;
		Uint32 i = (start + 1) % num_peers;
		while (i != start)
		{
			Peer* p = pman.getPeer(i);
			if (p && p->isChoked() && p->isInterested())
				return p->getID();
			i = (i + 1) % num_peers;
		}
		
		// we do not expect to have 4 billion peers
		return 0xFFFFFFFF;
	}
	
	//////////////////////////////////////////////
	
	int NChokeCmp(Peer* a,Peer* b)
	{
		Uint32 now = GetCurrentTime();
		// if they have pending upload requests or they were unchoked in the last 20 seconds, 
		// they are category 1 
		bool a_first_class = a->getPeerUploader()->getNumRequests() > 0 || 
				(now - a->getUnchokeTime() <= 20000);
		bool b_first_class = b->getPeerUploader()->getNumRequests() > 0 || 
				(now - b->getUnchokeTime() <= 20000);
		
		if (a_first_class && !b_first_class)
		{
			// category 1 come first
			return -1;
		}
		else if (!a_first_class && b_first_class)
		{
			// category 1 come first
			return 1;
		}
		else
		{
			// use upload rate to differentiate peers of the same class
			if (a->getUploadRate() > b->getUploadRate())
				return -1;
			else if (b->getUploadRate() > a->getUploadRate())
				return 1;
			else
				return 0;
		}
	}
	
	void NewChokeAlgorithm::doChokingSeederState(PeerManager& pman)
	{
		Uint32 num_peers = pman.getNumConnectedPeers();
		if (num_peers == 0)
			return;
		
		// first get all unchoked and interested peers
		PeerPtrList peers;
		for (Uint32 i = 0;i < num_peers;i++)
		{
			Peer* p = pman.getPeer(i);
			if (p && !p->isChoked() && p->isInterested())
				peers.append(p);
		}
		
		// sort them
		peers.setCompareFunc(NChokeCmp);
		peers.sort();
		
		// first round so take the 4 first peers
		if (round_state == 1)
		{
			for (Uint32 i = 0;i < peers.count();i++)
			{
				Peer* p = peers.at(i);
				if (!p)
					continue;
				
				if (i < 4)
					p->getPacketWriter().sendUnchoke();
				else
					p->getPacketWriter().sendChoke();
			}
		}
		else
		{
			Uint32 rnd = 0;
			if (peers.count() > 3)
				rnd = 3 + rand() % (peers.count() - 3); 
			
			// take the first 3 and a random one
			for (Uint32 i = 0;i < peers.count();i++)
			{
				Peer* p = peers.at(i);
				if (!p)
					continue;
				
				if (i < 3 || i == rnd)
					p->getPacketWriter().sendUnchoke();
				else 
					p->getPacketWriter().sendChoke();
			}
		}
		
		round_state++;
		if (round_state > 3)
			round_state = 1;
	}
	
	

}
