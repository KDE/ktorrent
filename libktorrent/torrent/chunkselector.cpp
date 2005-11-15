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
#include <stdlib.h>
#include <util/log.h>
#include <util/bitset.h>
#include "chunkcounter.h"
#include "chunkselector.h"
#include "chunkmanager.h"
#include "downloader.h"
#include "peerdownloader.h"
#include "globals.h"
#include "peer.h"
#include "peermanager.h"

namespace bt
{

	ChunkSelector::ChunkSelector(ChunkManager & cman,Downloader & downer,PeerManager & pman)
	: cman(cman),downer(downer),pman(pman)
	{
	}


	ChunkSelector::~ChunkSelector()
	{}

	bool ChunkSelector::findPriorityChunk(PeerDownloader* pd,Uint32 & chunk)
	{
		const BitSet & bs = cman.getBitSet();
		
		Uint32 i = 0;
		while (i < cman.getNumChunks())
		{
			Chunk* c = cman.getChunk(i);
			if (c->isPriority() && !c->isExcluded() && pd->hasChunk(i) &&
				!downer.areWeDownloading(i) && !bs.get(i))
			{
				chunk = i;
				return true;
			}
			i++;
		}
		return false;
	}

	bool ChunkSelector::select(PeerDownloader* pd,Uint32 & chunk)
	{
		// first try to find priority chunks
		if (findPriorityChunk(pd,chunk))
			return true;
		
		// cap the maximum chunk to download
		// to not get monster writes to the cache file
		Uint32 max_c = cman.getMaxAllowedChunk();
		if (max_c > cman.getNumChunks())
			max_c = cman.getNumChunks();

		const BitSet & bs = cman.getBitSet();
		Uint32 rarest_chunk=0xFFFFFFFF;
		Uint32 rarest_peer_cnt=0x7FFFFFFF;
		bool warmup = cman.getNumChunks() - cman.chunksLeft() <= 4;
		
		// pick a random chunk to download, by picking
		// a random starting value in the range 0 .. max_c
		Uint32 s = int(((double)rand() / (RAND_MAX - 1)) * max_c);
		Uint32 i = s;
		i = (i + 1) % max_c;
		// first try the range 0 .. max_c
		while (i != s)
		{
			Chunk* c = cman.getChunk(i);
			// pd has to have the selected chunk
			// and we don't have it
			if (pd->hasChunk(i) && !downer.areWeDownloading(i) &&
				!bs.get(i) && !c->isExcluded())
			{
				// find out how many peers already have the chunk			
				Uint32 peer_cnt = pman.getChunkCounter().get(i);
				Uint32 peers = pman.getNumConnectedPeers();
				if (warmup) 
				{
					// if in warmup mode, select the chunk 
					// 1/2 of the peers (approx) has .
					if (abs(int(peer_cnt)-int(peers/2))<
						abs(int(rarest_peer_cnt)-int(peers/2)))
					{
						rarest_chunk=i;
						rarest_peer_cnt=peer_cnt;
						if (abs(int(rarest_peer_cnt)-int(peers/2))<=1)
							break;
					}
				} 
				else if (peer_cnt<rarest_peer_cnt) 
				{
					// normal mode - select rarest chunk
					rarest_chunk=i;
					rarest_peer_cnt=peer_cnt;
					if (rarest_peer_cnt==1) break;
				}
			}
			i = (i + 1) % max_c;
		}

		if (rarest_chunk!=0xFFFFFFFF) {
			chunk=rarest_chunk;
			return true;
		} 

		// then try everything else
		for (i = max_c;i < cman.getNumChunks();i++)
		{
			Chunk* c = cman.getChunk(i);
			// pd has to have the selected chunk
			// and we don't have it
			if (pd->hasChunk(i) && !downer.areWeDownloading(i) &&
						 !bs.get(i) && !c->isExcluded())
			{
				chunk = i;
				return true;
			}
		}

		return false;
	}

	




}

