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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <algorithm>
#include "bitset.h"

namespace bt
{

	BitSet::BitSet(Uint32 num_bits) : num_bits(num_bits),data(0)
	{
		num_bytes = (num_bits / 8) + ((num_bits % 8 > 0) ? 1 : 0);
		data = new Uint8[num_bytes];
		std::fill(data,data+num_bytes,0x00);
	}

	BitSet::BitSet(const Uint8* d,Uint32 num_bits)  : num_bits(num_bits),data(0)
	{
		num_bytes = (num_bits / 8) + ((num_bits % 8 > 0) ? 1 : 0);
		data = new Uint8[num_bytes];
		std::copy(d,d+num_bytes,data);
	}
	
	BitSet::BitSet(const BitSet & bs) : num_bits(bs.num_bits),num_bytes(bs.num_bytes),data(0)
	{
		data = new Uint8[num_bytes];
		std::copy(bs.data,bs.data+num_bytes,data);
	}
			
	BitSet::~BitSet()
	{
		delete [] data;
	}

	bool BitSet::get(Uint32 i) const
	{
		if (i >= num_bits)
			return false;
		
		Uint32 byte = i / 8;
		Uint32 bit = i % 8;
		return data[byte] & (0x01 << (7 - bit));
	}
	
	void BitSet::set(Uint32 i,bool on)
	{
		if (i >= num_bits)
			return;
		
		Uint32 byte = i / 8;
		Uint32 bit = i % 8;
		if (on)
		{
			data[byte] |= (0x01 << (7 - bit));
		}
		else
		{
			Uint8 b = (0x01 << (7 - bit));
			data[byte] &= (~b);
		}
	}

	BitSet & BitSet::operator = (const BitSet & bs)
	{
		if (data)
			delete [] data;
		num_bytes = bs.num_bytes;
		num_bits = bs.num_bits;
		data = new Uint8[num_bytes];
		std::copy(bs.data,bs.data+num_bytes,data);
		return *this;
	}
}
;
