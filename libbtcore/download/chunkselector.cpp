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
#include "chunkselector.h"
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <util/log.h>
#include <util/bitset.h>
#include <peer/chunkcounter.h>
#include <diskio/chunkmanager.h>
#include <interfaces/piecedownloader.h>
#include <peer/peer.h>
#include <peer/peermanager.h>
#include <torrent/torrent.h>
#include "downloader.h"
#include "chunkdownload.h"

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
	: ChunkSelectorInterface(cman,downer,pman)
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
	
	Uint32 ChunkSelector::leastPeers(const std::list<Uint32> & lp,Uint32 alternative,Uint32 max_peers_per_chunk)
	{
		bool endgame = downer.endgameMode();
		
		Uint32 sel = lp.front();
		Uint32 cnt = downer.numDownloadersForChunk(sel);
		for (std::list<Uint32>::const_iterator i = lp.begin();i != lp.end();i++)
		{
			Uint32 cnt_i = downer.numDownloadersForChunk(*i);
			if (cnt_i < cnt)
			{
				sel = *i;
				cnt = cnt_i;
			}
		}
		
		if (!endgame && downer.numDownloadersForChunk(sel) >= max_peers_per_chunk)
		{
			ChunkDownload* cd = downer.getDownload(sel);
			if (!cd)
				return alternative;
			
			// if download speed is very small, use sel
			// even though the max_peers_per_chunk has been reached
			if (cd->getDownloadSpeed() < 100)
				return sel;
			else
				return alternative;
		}
		
		return sel;
	}

	bool ChunkSelector::select(PieceDownloader* pd,Uint32 & chunk)
	{		
		const BitSet & bs = cman.getBitSet();
		
		std::list<Uint32> preview;
		std::list<Uint32> normal;
		std::list<Uint32> first;
		Uint32 sel = cman.getNumChunks() + 1;
		
		// sort the chunks every 2 seconds
		if (sort_timer.getElapsedSinceUpdate() > 2000)
		{
			bool warmup = cman.getNumChunks() - cman.chunksLeft() <= 4;
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
				if (pd->hasChunk(i) && !c->isExcluded() && !c->isExcludedForDownloading())
				{
					if (!downer.areWeDownloading(i))
					{ 
						// we have a chunk
						sel = i;
						break;
					}
					
					switch (cman.getChunk(i)->getPriority())
					{
						case PREVIEW_PRIORITY:
							preview.push_back(i);
							break;
						case FIRST_PRIORITY:
							first.push_back(i);
							break;
						case NORMAL_PRIORITY:
							normal.push_back(i);
							break;
						default:
							break;
					}
					
				}
				itr++;
			}
		}
		
		if (sel >= cman.getNumChunks())
			return false;
		
		// we have found one, now try to see if we cannot assign this PieceDownloader to a higher priority chunk
		switch (cman.getChunk(sel)->getPriority())
		{
			case PREVIEW_PRIORITY:
				chunk = sel;
				return true;
			case FIRST_PRIORITY:
				if (preview.size() > 0)
				{
					chunk = leastPeers(preview,sel,3);
					return true;
				}
				else
				{
					chunk = sel;
					return true;
				}
				break;
			case NORMAL_PRIORITY:
				if (preview.size() > 0)
				{
					chunk = leastPeers(preview,sel,3);
					return true;
				}
				else if (first.size() > 0)
				{
					chunk = leastPeers(first,sel,2);
					return true;
				}
				else
				{
					chunk = sel;
					return true;
				}
				break;
			case LAST_PRIORITY:
				if (preview.size() > 0)
				{
					chunk = leastPeers(preview,sel,3);
					return true;
				}
				else if (first.size() > 0)
				{
					chunk = leastPeers(first,sel,2);
					return true;
				}
				else if (normal.size() > 0)
				{
					chunk = leastPeers(normal,sel,2);
					return true;
				}
				else
				{
					chunk = sel;
					return true;
				}
				break;
			default:
				chunk = sel;
				return true;
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

	const Uint32 MAX_RANGE_SIZE = 10 * 1024 * 1024; // lets take 10 MB for the max range to download in one go 
	
	struct ChunkRange
	{
		Uint32 start;
		Uint32 len;
	};
	
	bool ChunkSelector::selectRange(Uint32 & from,Uint32 & to,Uint32 max_len)
	{
		Uint32 max_range_len = max_len;
		
		const BitSet & bs = cman.getBitSet();
		Uint32 num_chunks = cman.getNumChunks();
		ChunkRange curr = {0,0};
		ChunkRange best = {0,0};
		
		for (Uint32 i = 0;i < num_chunks;i++)
		{
			if (!bs.get(i) && downer.canDownloadFromWebSeed(i))
			{
				if (curr.start == 0 && curr.len == 0) // we have a new current range
				{
					curr.start = i;
					curr.len = 1;
				}
				else
					curr.len++; // expand the current range
				
				if (curr.len >= max_range_len)
				{
					// current range is bigger then the maximum range length, select it
					from = curr.start;
					to = curr.start + curr.len - 1;
					return true;
				}
				else if (i == num_chunks - 1)
				{
					if (curr.start > 0 && curr.len > best.len)
						best = curr;
				}
			}
			else
			{
				// see if the current range has ended
				// and if it is better then the best range
				if (curr.start > 0 && curr.len > best.len)
					best = curr; // we have a new best range
				
				curr.start = curr.len = 0; // reset the current range
			}
		}
		
		// end of the list, so select the best
		if (best.len)
		{
			from = best.start;
			to = best.start + best.len - 1;
			return true;
		}
		return false;
	}
}

