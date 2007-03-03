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
#if 0
#ifndef BTNEWCHOKEALGORITHM_H
#define BTNEWCHOKEALGORITHM_H

#include <choker.h>

namespace bt
{

	/**
	 * @author Joris Guisson
	 * 
	 * The new choking algorithm.
	*/
	class NewChokeAlgorithm : public ChokeAlgorithm
	{
		Uint32 round_state;
	public:
		NewChokeAlgorithm();
		virtual ~NewChokeAlgorithm();

		virtual void doChokingLeechingState(PeerManager & pman,ChunkManager & cman,const kt::TorrentStats & stats);
		virtual void doChokingSeedingState(PeerManager & pman,ChunkManager & cman,const kt::TorrentStats & stats);
	private:
		void doChokingLeecherState(PeerManager& pman);
		void doChokingSeederState(PeerManager& pman);
	
		Uint32 findPlannedOptimisticUnchokedPeer(PeerManager& pman);
	};

}

#endif
#endif

