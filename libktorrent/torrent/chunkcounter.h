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
#ifndef BTCHUNKCOUNTER_H
#define BTCHUNKCOUNTER_H

#include <util/constants.h>
#include <util/array.h>

namespace bt 
{
	class BitSet;

	/**
	 * @author Joris Guisson
	 * 
	 * Class to keep track of how many peers have a chunk.
	*/
	class ChunkCounter 
	{
		Array<Uint32> cnt;
	public:
		ChunkCounter(Uint32 num_chunks);
		virtual ~ChunkCounter();
	
		/**
		 * If a bit in the bitset is one, increment the corresponding counter.
		 * @param bs The BitSet
		 */
		void incBitSet(const BitSet & bs);
		
		
		/**
		 * If a bit in the bitset is one, decrement the corresponding counter.
		 * @param bs The BitSet
		 */
		void decBitSet(const BitSet & bs);
		
		/**
		 * Increment the counter for the idx'th chunk 
		 * @param idx Index of the chunk
		 */
		void inc(Uint32 idx);
		
		
		/**
		 * Decrement the counter for the idx'th chunk 
		 * @param idx Index of the chunk
		 */
		void dec(Uint32 idx);
		
		
		/**
		 * Get the counter for the idx'th chunk 
		 * @param idx Index of the chunk
		 */
		Uint32 get(Uint32 idx) const;
		
		/**
		 * Reset all values to 0
		 */
		void reset();
	};

}

#endif
