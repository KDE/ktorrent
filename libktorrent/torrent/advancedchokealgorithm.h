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
#ifndef BTADVANCEDCHOKEALGORITHM_H
#define BTADVANCEDCHOKEALGORITHM_H

#include "choker.h"

namespace bt
{
	class Peer;
	class PeerPtrList;
	

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class AdvancedChokeAlgorithm : public ChokeAlgorithm
	{
		TimeStamp last_opt_sel_time; // last time we updated the optimistic unchoked peer
	public:
		AdvancedChokeAlgorithm();
		virtual ~AdvancedChokeAlgorithm();

		virtual void doChokingLeechingState(PeerManager & pman,ChunkManager & cman,const kt::TorrentStats & stats);
		virtual void doChokingSeedingState(PeerManager & pman,ChunkManager & cman,const kt::TorrentStats & stats);
		
	private:
		bool calcACAScore(Peer* p,ChunkManager & cman,const kt::TorrentStats & stats);
		Peer* updateOptimisticPeer(PeerManager & pman,const PeerPtrList & ppl);
		void doUnchoking(PeerPtrList & ppl,Peer* poup);
	};

}

#endif
