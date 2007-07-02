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
#include <algorithm>
#include "bitset.h"
#include <string.h>

namespace bt
{
	BitSet BitSet::null;

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

	

	BitSet & BitSet::operator = (const BitSet & bs)
	{
		if (data)
			delete [] data;
		num_bytes = bs.num_bytes;
		num_bits = bs.num_bits;
		data = new Uint8[num_bytes];
		std::copy(bs.data,bs.data+num_bytes,data);
		num_on = bs.num_on;
		return *this;
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
		while (i < num_bits)
		{
			bool val = get(i) || other.get(i);
			set(i,val);
			i++;
		}
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

