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
#include <util/bitset.h>
#include "chunkcounter.h"

namespace bt 
{

	ChunkCounter::ChunkCounter(Uint32 num_chunks) : cnt(num_chunks)
	{
		// fill with 0
		cnt.fill(0);
	}
	
	
	ChunkCounter::~ChunkCounter()
	{
	}
	
	void ChunkCounter::reset()
	{
		cnt.fill(0);
	}
	
	void ChunkCounter::incBitSet(const BitSet & bs)
	{
		for (Uint32 i = 0;i < cnt.size();i++)
		{
			if(bs.get(i))
				cnt[i]++;
		}
	}
	
	void ChunkCounter::decBitSet(const BitSet & bs)
	{
		for (Uint32 i = 0;i < cnt.size();i++)
		{
			if(bs.get(i))
				dec(i);
		}
	}

	void ChunkCounter::inc(Uint32 idx)
	{
		if (idx < cnt.size())
			cnt[idx]++;
	}
		
	void ChunkCounter::dec(Uint32 idx)
	{
		if (idx < cnt.size() && cnt[idx] > 0)
			cnt[idx]--;
	}
		
	Uint32 ChunkCounter::get(Uint32 idx) const
	{
		if (idx < cnt.size())
			return cnt[idx];
		else
			return 0;
	}

}
