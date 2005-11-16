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
#include "newchokealgorithm.h"
#include "peermanager.h"

namespace bt
{

	NewChokeAlgorithm::NewChokeAlgorithm(): ChokeAlgorithm()
	{}


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
	
	void NewChokeAlgorithm::doChokingLeecherState(PeerManager& pman)
	{
		PeerPtrList downloaders;
		PeerPtrList master;
	}
	
	void NewChokeAlgorithm::doChokingSeederState(PeerManager& pman)
	{
	}
	
#if 0	
	int NChokeCmp(Peer* a,Peer* b)
	{
		Uint32 now = GetCurrentTime();
		bool a_first_class = a->getPeerDownloader()->getNumRequests() || 
				(now - a->getChokeTime() <= 20000);
		bool b_first_class = b->getPeerDownloader()->getNumRequests() || 
				(now - b->getChokeTime() <= 20000);
		// if they have pending download requests or they were unchoked in the last 20 seconds, 
		// they are category 1 
		
		if (a_first_class && b_first_class)
		{
		}
		else if (a_first_class && !b_first_class)
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
			// both category 2, use download rate
		}
	}

	void NewChokeAlgorithm::fillMaster(PeerManager& pman)
	{
		
		master.clear();
		// first update it
		for (Uint32 i = 0;i < pman.getNumConnectedPeers();i++)
		{
			Peer* p = pman.getPeer(i);
			// only add the unchoked and interested peers
			if (!p->isChoked() && !p->isInterested())
				master.append(p);
		}
		
		// now sort them
		master.setCompareFunc(NChokeCmp);
		master.sort();
	}
#endif
}
