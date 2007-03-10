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
#include <stdlib.h>
#include <util/functions.h>
#include <interfaces/torrentinterface.h>
#include "chunkmanager.h"
#include "peer.h"
#include "peermanager.h"
#include "packetwriter.h"
#include "advancedchokealgorithm.h"

using namespace kt;

namespace bt
{
	
	
	const Uint32 OPT_SEL_INTERVAL = 30*1000; // we switch optimistic peer each 30 seconds
	const double NEWBIE_BONUS = 1.0;
	const double SNUB_PENALTY = 10.0;
	const double ONE_MB = 1024*1024;
		

	AdvancedChokeAlgorithm::AdvancedChokeAlgorithm()
			: ChokeAlgorithm()
	{
		last_opt_sel_time = 0;
	}


	AdvancedChokeAlgorithm::~AdvancedChokeAlgorithm()
	{}
	
	bool AdvancedChokeAlgorithm::calcACAScore(Peer* p,ChunkManager & cman,const kt::TorrentStats & stats)
	{
		const PeerInterface::Stats & s = p->getStats();
		if (p->isSeeder())
		{
			/*
			double bd = 0;
			if (stats.trk_bytes_downloaded > 0)
			 	bd = s.bytes_downloaded / stats.trk_bytes_downloaded;
			double ds = 0;
			if (stats.download_rate > 0)
				ds = s.download_rate/ stats.download_rate;
			p->setACAScore(5*bd + 5*ds);
			*/
			p->setACAScore(0.0);
			return false;
		}
		
		bool should_be_interested = false;
		bool should_we_be_interested = false;
		// before we start calculating first check if we have piece that the peer doesn't have
		const BitSet & ours = cman.getBitSet();
		const BitSet & theirs = p->getBitSet();
		for (Uint32 i = 0;i < ours.getNumBits();i++)
		{
			if (ours.get(i) && !theirs.get(i))
			{
				should_be_interested = true;
				break;
			}
		}
		
		if (!should_be_interested || !p->isInterested())
		{
			// not interseted so it doesn't make sense to unchoke it
			p->setACAScore(-50.0);
			return false;
		}

				
		
		double nb = 0.0; // newbie bonus
		double cp = 0.0; // choke penalty
		double sp = 0.0; // snubbing penalty
		double lb = s.local ? 10.0 : 0.0; // local peers get a bonus of 10
		double bd = s.bytes_downloaded; // bytes downloaded
		double tbd = stats.trk_bytes_downloaded; // total bytes downloaded
		double ds = s.download_rate; // current download rate
		double tds = stats.download_rate; // total download speed
		
		// if the peer has less than 1 MB or 0.5 % of the torrent it is a newbie
		if (p->percentAvailable() < 0.5 && stats.total_bytes * p->percentAvailable() < 1024*1024)
		{
			nb = NEWBIE_BONUS;
		}
		
		if (p->isChoked())
		{
			cp = NEWBIE_BONUS; // cp cancels out newbie bonus
		}
		
		// if the evil bit is on (!choked, snubbed and requests have timed out)
		if (s.evil)
		{
			sp = SNUB_PENALTY;
		}
		
		// NB + K * (BD/TBD) - CP - SP + L * (DS / TDS)
		double K = 5.0;
		double L = 5.0;
		double aca = lb + nb + (tbd > 0 ? K * (bd/tbd) : 0.0) + (tds > 0 ? L* (ds / tds) : 0.0) - cp - sp;
		
		p->setACAScore(aca);
		return true;
	}
	
	static int ACACmp(Peer* a,Peer* b)
	{
		if (a->getStats().aca_score < b->getStats().aca_score)
			return 1;
		else if (a->getStats().aca_score > b->getStats().aca_score)
			return -1;
		else
			return 0;
	}
	

	void AdvancedChokeAlgorithm::doChokingLeechingState(PeerManager & pman,ChunkManager & cman,const kt::TorrentStats & stats)
	{
		PeerPtrList ppl;
		Uint32 np = pman.getNumConnectedPeers();
		// add all non seeders
		for (Uint32 i = 0;i < np;i++)
		{
			Peer* p = pman.getPeer(i);
			if (p)
			{
				if (calcACAScore(p,cman,stats))
					ppl.append(p);
				else
					// choke seeders they do not want to download from us anyway
					p->choke();
			}
		}
		
		// sort list by ACA score
		ppl.setCompareFunc(ACACmp); 
		ppl.sort();
		
		doUnchoking(ppl,updateOptimisticPeer(pman,ppl));
	}
	
	void AdvancedChokeAlgorithm::doUnchoking(PeerPtrList & ppl,Peer* poup)
	{
		// Get the number of upload slots
		Uint32 num_slots = Choker::getNumUploadSlots();
		// Do the choking and unchoking
		Uint32 num_unchoked = 0;
		for (Uint32 i = 0;i < ppl.count();i++)
		{
			Peer* p = ppl.at(i);
			if (!poup && num_unchoked < num_slots)
			{
				p->getPacketWriter().sendUnchoke();
				num_unchoked++;
			}
			else if (num_unchoked < num_slots -1 || p == poup)
			{
				p->getPacketWriter().sendUnchoke();
				if (p != poup)
					num_unchoked++;
			}
			else
			{
				p->choke();
			}
		}
	}
	
	static int UpRateCmp(Peer* a,Peer* b)
	{
		if (a->getStats().upload_rate < b->getStats().upload_rate)
			return -1;
		else if (a->getStats().upload_rate > b->getStats().upload_rate)
			return 1;
		else
			return 0;
	}

	void AdvancedChokeAlgorithm::doChokingSeedingState(PeerManager & pman,ChunkManager & cman,const kt::TorrentStats & stats)
	{
		PeerPtrList ppl;
		Uint32 np = pman.getNumConnectedPeers();
		// add all non seeders
		for (Uint32 i = 0;i < np;i++)
		{
			Peer* p = pman.getPeer(i);
			if (p)
			{
				// update the ACA score in the process
				if (calcACAScore(p,cman,stats))
					ppl.append(p);
				else
					// choke seeders they do not want to download from us anyway
					p->choke();  
			}
		}
		
		ppl.setCompareFunc(UpRateCmp);
		ppl.sort();
		
		doUnchoking(ppl,updateOptimisticPeer(pman,ppl));
	}
	
	static Uint32 FindPlannedOptimisticUnchokedPeer(PeerManager& pman,const PeerPtrList & ppl)
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
			if (p && p->isChoked() && p->isInterested() && !p->isSeeder() && ppl.contains(p))
				return p->getID();
			i = (i + 1) % num_peers;
		}
		
		// we do not expect to have 4 billion peers
		return UNDEFINED_ID;
	}
	
	Peer* AdvancedChokeAlgorithm::updateOptimisticPeer(PeerManager & pman,const PeerPtrList & ppl)
	{
		// get the planned optimistic unchoked peer and change it if necessary
		Peer* poup = pman.findPeer(opt_unchoked_peer_id);
		TimeStamp now = GetCurrentTime();
		if (now - last_opt_sel_time > OPT_SEL_INTERVAL || !poup)
		{
			opt_unchoked_peer_id = FindPlannedOptimisticUnchokedPeer(pman,ppl);
			last_opt_sel_time = now;
			poup = pman.findPeer(opt_unchoked_peer_id);
		}
		return poup;
	}
}
