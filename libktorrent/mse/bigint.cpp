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
#include <string.h>
#include "bigint.h"

namespace mse
{


	BigInt::BigInt(Uint32 num_bits) : data(0),num_bits(num_bits),num_bytes(0)
	{
		if (num_bits > 0)
		{
			num_bytes = num_bits / 8;
			if (num_bytes % 8 > 0)
				num_bytes++;
			data = new Uint8[num_bytes];
			memset(data,0,num_bytes);
		}
	}
		
	BigInt::BigInt(const QString & value) : data(0),num_bits(0),num_bytes(0)
	{
		// each letter is a nibble
		num_bits = value.length() * 4;
		if (num_bits > 0)
		{
			num_bytes = num_bits / 8;
			if (num_bytes % 8 > 0)
				num_bytes++;
			
			Uint8 h = 0x00,l = 0x00;
			bool full_byte = false;
			Uint32 cb = num_bytes - 1;
			for (Uint32 i = 0;i < value.length();i++)
			{
				Uint8 nibble = 0x00;
				char c = value[i];
				// first convert the char to a nibble
				if (c >= '0' && c <= '9')
					nibble = c - '0';
				else if (c >= 'a' && c <= 'f')
					nibble = c - 'a';
				else if (c >= 'A' && c <= 'F')
					nibble = c - 'A';
				else
				{
					// not a valid char
					delete [] data;
					data = 0;
					num_bits = 0;
					return;
				}
				
				if (!full_byte)
				{
					// save nibble in h
					h = (nibble << 4) & 0xF0;
					full_byte = true;
				}
				else
				{
					// we have a full byte
					l = nibble & 0x0F;
					full_byte = false;
					data[cb--] = h & l; // and h and l nibble to get the whole byte
					h = l = 0x00; 
				}
			}
			
			// add residual nibble
			if (full_byte)
				data[cb--] = h >> 4; 
			// store in the lower bits so that XOR with a higher number of bits BigInt will work as expected
		}
	}
		
	BigInt::BigInt(const BigInt & bi) : data(0),num_bits(bi.num_bits),num_bytes(0)
	{
		if (num_bits > 0)
		{
			num_bytes = num_bits / 8;
			if (num_bytes % 8 > 0)
				num_bytes++;
			data = new Uint8[num_bytes];
			memcpy(data,bi.data,num_bytes);
		}
	}
	
	BigInt::~BigInt()
	{
		delete [] data;
	}
		
	BigInt & BigInt::operator = (const BigInt & bi)
	{
		if (data)
			delete [] data;
		
		num_bits = bi.num_bits;
		if (num_bits > 0)
		{
			num_bytes = num_bits / 8;
			if (num_bytes % 8 > 0)
				num_bytes++;
			data = new Uint8[num_bytes];
			memcpy(data,bi.data,num_bytes);
		}
		else
		{
			data = 0;
		}
		return *this;
	}
	
	BigInt BigInt::mod(const BigInt & a,const BigInt & b)
	{
		return BigInt(0);
	}
		
	BigInt BigInt::exclusiveOr(const BigInt & a,const BigInt & b)
	{
		Uint32 num_bits = a.num_bits > b.num_bits ? a.num_bits : b.num_bits;
		BigInt r(num_bits);
		
		// calculate number of bytes
		Uint32 num_bytes = num_bits / 8;
		if (num_bytes % 8 > 0)
			num_bytes++;
		
		// now just xor all bytes
		for (Uint32 i = 0;i < num_bytes;i++)
		{
			Uint8 ab = i < a.num_bytes ? a.data[i] : 0x00;
			Uint8 bb = i < b.num_bytes ? b.data[i] : 0x00;
			r.data[i] = ab ^ bb;
		}
		
		return r;
	}

}
