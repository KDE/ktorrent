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
#include "bitset.h"
#include <algorithm>
#include <string.h>

namespace bt
{
	BitSet BitSet::null;
	
	// Fast lookup table to see how many bits are there in a byte
	static const Uint8 BitCount[] = 
	{
		0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
	};


	BitSet::BitSet(Uint32 num_bits) : num_bits(num_bits),data(0)
	{
		num_bytes = (num_bits / 8) + ((num_bits % 8 > 0) ? 1 : 0);
		data = new Uint8[num_bytes];
		std::fill(data,data+num_bytes,0x00);
		num_on = 0;
	}

	BitSet::BitSet(const Uint8* d,Uint32 num_bits)  : num_bits(num_bits),data(0)
	{
		num_bytes = (num_bits / 8) + ((num_bits % 8 > 0) ? 1 : 0);
		data = new Uint8[num_bytes];
		memcpy(data,d,num_bytes);
		num_on = 0;
		Uint32 i = 0;
		while (i < num_bits)
		{
			if (get(i))
				num_on++;
			i++;
		}
	}
	
	BitSet::BitSet(const BitSet & bs) : num_bits(bs.num_bits),num_bytes(bs.num_bytes),data(0),num_on(bs.num_on)
	{
		data = new Uint8[num_bytes];
		std::copy(bs.data,bs.data+num_bytes,data);
	}
			
	BitSet::~BitSet()
	{
		delete [] data;
	}

	void BitSet::updateNumOnBits()
	{
		num_on = 0;
		Uint32 i = 0;
		while (i < num_bytes)
		{
			num_on += BitCount[data[i]];
			i++;
		}
	}

	BitSet & BitSet::operator = (const BitSet & bs)
	{
		delete [] data;
		num_bytes = bs.num_bytes;
		num_bits = bs.num_bits;
		data = new Uint8[num_bytes];
		std::copy(bs.data,bs.data+num_bytes,data);
		num_on = bs.num_on;
		return *this;
	}
	
	void BitSet::invert()
	{
		Uint32 i = 0;
		while (i < num_bits)
		{
			set(i,!get(i));
			i++;
		}
	}
	
	BitSet & BitSet::operator -= (const BitSet & bs)
	{
		Uint32 i = 0;
		while (i < num_bits)
		{
			if (get(i) && bs.get(i))
				set(i,false);
			i++;
		}
		return *this;
	}

	BitSet & BitSet::operator - (const BitSet & bs)
	{
		return BitSet(*this) -= bs;
	}
	
	void BitSet::setAll(bool on)
	{
		std::fill(data,data+num_bytes,on ? 0xFF : 0x00);
		num_on = on ? num_bits : 0;
	}

	void BitSet::clear()
	{
		setAll(false);
	}

	void BitSet::orBitSet(const BitSet & other)
	{		
		Uint32 i = 0;
		num_on = 0;
		while (i < num_bytes)
		{
			Uint8 od = 0;
			if (i < other.num_bytes)
				od = other.data[i];
			data[i] |= od;
			num_on += BitCount[data[i]];
			i++;
		}
	}
	
	void BitSet::andBitSet(const BitSet & other)
	{
		Uint32 i = 0;
		num_on = 0;
		while (i < num_bytes)
		{
			Uint8 od = 0;
			if (i < other.num_bytes)
				od = other.data[i];
			data[i] &= od;
			num_on += BitCount[data[i]];
			i++;
		}
	}

	bool BitSet::includesBitSet(const BitSet & other)
	{
		Uint32 i = 0;
		while (i < num_bits)
		{
			if (other.get(i) && !get(i))
				return false;
			i++;
		}
		return true;
	}

	bool BitSet::allOn() const
	{
		return num_on == num_bits;
	}

	bool BitSet::operator == (const BitSet & bs)
	{
		if (this->getNumBits() != bs.getNumBits())
			return false;

		return memcmp(data,bs.data,num_bytes) == 0;
	}
}

