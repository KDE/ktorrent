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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "choker.h"
#include "advancedchokealgorithm.h"
#include <util/functions.h>
#include <peer/peermanager.h>

namespace bt
{
	
	ChokeAlgorithm::ChokeAlgorithm() : opt_unchoked_peer_id(0)
	{
	}
	
	ChokeAlgorithm::~ChokeAlgorithm()
	{
	}
	
	
	/////////////////////////////////
	
	Uint32 Choker::num_upload_slots = 2;

	Choker::Choker(PeerManager & pman,ChunkManager & cman) : pman(pman),cman(cman)
	{
		choke = new AdvancedChokeAlgorithm();
	}


	Choker::~Choker()
	{
		delete choke;
	}

	void Choker::update(bool have_all,const TorrentStats & stats)
	{
		if (have_all)
			choke->doChokingSeedingState(pman,cman,stats);
		else
			choke->doChokingLeechingState(pman,cman,stats);
	}
	
}
