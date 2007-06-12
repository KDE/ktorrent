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
#include <vector>
#include <algorithm>
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
	struct RareCmp
	{
		ChunkManager & cman;
		ChunkCounter & cc;
		bool warmup;
		
		RareCmp(ChunkManager & cman,ChunkCounter & cc,bool warmup) : cman(cman),cc(cc),warmup(warmup) {}
		
		bool operator()(Uint32 a,Uint32 b)
		{
			// do some sanity checks 
			if (a >= cman.getNumChunks() || b >= cman.getNumChunks())
				return false;
			
			// the sorting is done on two criteria, priority and rareness
			Priority pa = cman.getChunk(a)->getPriority();
			Priority pb = cman.getChunk(b)->getPriority();
			if (pa == pb)
				return normalCmp(a,b); // if both have same priority compare on rareness
			else if (pa > pb) // pa has priority over pb, so select pa
				return true;
			else // pb has priority over pa, so select pb
				return false;
		}
		
		bool normalCmp(Uint32 a,Uint32 b)
		{
			// during warmup mode choose most common chunks
			if (!warmup)
				return cc.get(a) < cc.get(b);
			else
				return cc.get(a) > cc.get(b);
		}
	};

	ChunkSelector::ChunkSelector(ChunkManager & cman,Downloader & downer,PeerManager & pman)
	: cman(cman),downer(downer),pman(pman)
	{	
		std::vector<Uint32> tmp;
		for (Uint32 i = 0;i < cman.getNumChunks();i++)
		{
			if (!cman.getBitSet().get(i))
			{
				tmp.push_back(i);
			}
		}
		std::random_shuffle(tmp.begin(),tmp.end());
		// std::list does not support random_shuffle so we use a vector as a temporary storage
		// for the random_shuffle
		chunks.insert(chunks.begin(),tmp.begin(),tmp.end());
		sort_timer.update();
	}


	ChunkSelector::~ChunkSelector()
	{}


	bool ChunkSelector::select(PeerDownloader* pd,Uint32 & chunk)
	{		
		const BitSet & bs = cman.getBitSet();
		
		
		// sort the chunks every 2 seconds
		if (sort_timer.getElapsedSinceUpdate() > 2000)
		{
			bool warmup = cman.getNumChunks() - cman.chunksLeft() <= 4;
//			dataChecked(bs);
			chunks.sort(RareCmp(cman,pman.getChunkCounter(),warmup));
			sort_timer.update();
		}
	
		std::list<Uint32>::iterator itr = chunks.begin();
		while (itr != chunks.end())
		{
			Uint32 i = *itr;
			Chunk* c = cman.getChunk(*itr);
			
			// if we have the chunk remove it from the list
			if (bs.get(i))
			{
				std::list<Uint32>::iterator tmp = itr;
				itr++;
				chunks.erase(tmp);
			}
			else
			{
				// pd has to have the selected chunk and it needs to be not excluded
				if (pd->hasChunk(i) && !downer.areWeDownloading(i) && 
					!c->isExcluded() && !c->isExcludedForDownloading())
				{
					// we have a chunk
					chunk = i;
					return true;
				}
				itr++;
			}
		}
		
		return false;
	}

	void ChunkSelector::dataChecked(const BitSet & ok_chunks)
	{
		for (Uint32 i = 0;i < ok_chunks.getNumBits();i++)
		{
			bool in_chunks = std::find(chunks.begin(),chunks.end(),i) != chunks.end();
			if (in_chunks && ok_chunks.get(i))
			{
				// if we have the chunk, remove it from the chunks list
				chunks.remove(i);
			}
			else if (!in_chunks && !ok_chunks.get(i))
			{
				// if we don't have the chunk, add it to the list if it wasn't allrready in there
				chunks.push_back(i);
			}
		}
	}

	void ChunkSelector::reincluded(Uint32 from, Uint32 to)
	{
		// lets do a safety check first
		if (from >= cman.getNumChunks() || to >= cman.getNumChunks())
		{
			Out(SYS_DIO|LOG_NOTICE) << "Internal error in chunkselector" << endl;
			return;
		}
		
		for (Uint32 i = from;i <= to;i++)
		{
			bool in_chunks = std::find(chunks.begin(),chunks.end(),i) != chunks.end();
			if (!in_chunks && cman.getChunk(i)->getStatus() != Chunk::ON_DISK)
			{
			//	Out(SYS_DIO|LOG_DEBUG) << "ChunkSelector::reIncluded " << i << endl;
				chunks.push_back(i);
			}
		}
	}
	
	void ChunkSelector::reinsert(Uint32 chunk)
	{
		bool in_chunks = std::find(chunks.begin(),chunks.end(),chunk) != chunks.end();
		if (!in_chunks)
			chunks.push_back(chunk);
	}


}

