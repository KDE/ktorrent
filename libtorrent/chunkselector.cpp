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
#include <libutil/log.h>
#include "bitset.h"
#include "chunkselector.h"
#include "chunkmanager.h"
#include "downloader.h"
#include "peerdownloader.h"
#include "globals.h"

namespace bt
{

	ChunkSelector::ChunkSelector(ChunkManager & cman,Downloader & downer)
	: cman(cman),downer(downer)
	{
	}


	ChunkSelector::~ChunkSelector()
	{}

	bool ChunkSelector::findPriorityChunk(PeerDownloader* pd,Uint32 & chunk)
	{
		BitSet bs;
		cman.toBitSet(bs);
		
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

		BitSet bs;
		cman.toBitSet(bs);
		

		// if need be we can increase max_c
		// when we don't find a suitable chunk 
		do 
		{
			// pick a random chunk to download, by picking
			// a random starting value in the range 0 .. max_c
			Uint32 s = int(((double)rand() / (RAND_MAX - 1)) * max_c);
			Uint32 i = s;
			
			do
			{
				Chunk* c = cman.getChunk(i);
				// pd has to have the selected chunk
				// and we don't have it
				if (pd->hasChunk(i) && !downer.areWeDownloading(i) &&
								!bs.get(i) && !c->isExcluded())
				{
					chunk = i;
				//	Out() << "Selecting " << i << endl;
					return true;
				}
				i = (i + 1) % max_c;
				// we stop this loop if i becomse equal to it's starting value
				// no infinite loops, thank you
			}while (s != i);
			max_c += 50;
			if (max_c > cman.getNumChunks())
				max_c = cman.getNumChunks();
			
		}while (max_c < cman.getNumChunks());

		//Out() << "Can't find chunk" << endl;

		return false;
	}

	




}
