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
#include <util/log.h>
#include <torrent/globals.h>
#include "bigint.h"

using namespace bt;

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
		if (num_bits == 0)
			return;
		
		
		num_bytes = num_bits / 8;
		if (num_bits % 8 > 0)
			num_bytes++;
		
	//	Out() << "num_bytes = " << num_bytes << endl;
		data = new Uint8[num_bytes];
		memset(data,0,num_bytes);
		
		Uint8 h = 0x00,l = 0x00;
		bool full_byte = value.length() % 2 == 1;
		Uint32 cb = 0;//num_bytes - 1;
		for (Uint32 i = 0;i < value.length();i++)
		{
			Uint8 nibble = 0x00;
			char c = value[i];
			// first convert the char to a nibble
			if (c >= '0' && c <= '9')
				nibble = c - '0';
			else if (c >= 'a' && c <= 'f')
				nibble = (c - 'a') + 10;
			else if (c >= 'A' && c <= 'F')
				nibble = (c - 'A') + 10;
			else
			{
				// not a valid char
				delete [] data;
				data = 0;
				num_bits = 0;
				return;
			}
			
			//Out() << QString("char : %1 ; nibble = %2").arg(c).arg(nibble,2,16) << endl;
			
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
			//	Out() << QString("0x%1").arg(h | l,2,16).upper() << endl;
				data[cb++] = h | l; // and h and l nibble to get the whole byte
				h = l = 0x00; 
			}
		}
			
#if 0
		Out() << "Value = " << value << endl;
		for (Uint32 i = 0;i < num_bytes;i++)
			Out() << QString("0x%1").arg(data[i],2,16).upper() << endl;
#endif 
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
	
	void BigInt::hexDump() const
	{
		QString t;
		for (Uint32 i = 0;i < num_bytes;i++)
			t += "0x" + QString("%1").arg(data[i],2,16).upper() + " ";
		Out() << t << endl;
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
		if (b > a)
			return a;
		
		if (b == a)
			return BigInt(0);
		
		
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
		
		int ai = a.num_bytes - 1;
		int bi = b.num_bytes - 1;
		// now just xor all bytes
		for (int i = (int)(num_bytes - 1);i >= 0;i--)
		{
			Uint8 ab = ai >= 0 ? a.data[ai] : 0x00;
			Uint8 bb = bi >= 0 ? b.data[bi] : 0x00;
			r.data[i] = ab ^ bb;
			ai--;
			bi--;
		}
		
		return r;
	}
	
	bool BigInt::isNull() const
	{
		if (num_bits == 0)
			return true;
		
		for (Uint32 i = 0;i < num_bytes;i++)
			if (data[i])
				return false;
		
		return true;
	}
	
	bool operator > (const BigInt & a,const BigInt & b)
	{
		if (a.isNull() && b.isNull())
			return false;
		else if (a.isNull() && !b.isNull())
			return false;
		else if (!a.isNull() && b.isNull())
			return true;
		else if (a.getNumBytes() == b.getNumBytes())
			return a.data[0] > b.data[0];
		else if (a.getNumBytes() < b.getNumBytes())
		{
			/*
				if 0x000EEF
				   0x123ABC
				The critical index is 3 (the position of A in the second number)
			*/
			Uint32 ci = b.getNumBytes() - a.getNumBytes();
			// now check if there is a byte before the critical index, which is > 0, this means B is bigger
			for (Uint32 i = 0;i < ci;i++)
				if (b.data[i] > 0)
					return false; // B is bigger
			
			// no byte bigger then 0 before, so we must compare the ci byte of B with the first byte of A
			return b.data[ci] < a.data[0];
		}
		else
		{
			/*
			if 0x000EEF
			0x123ABC
			The critical index is 3 (the position of A in the second number)
			*/
			Uint32 ci = a.getNumBytes() - b.getNumBytes();
			// now check if there is a byte before the critical index, which is > 0, this means A is bigger
			for (Uint32 i = 0;i < ci;i++)
				if (a.data[i] > 0)
					return true; // A is bigger
			
			// no byte bigger then 0 before, so we must compare the ci byte of A with the first byte of B
			return a.data[ci] > b.data[0];
		}
	}
	
	bool operator == (const BigInt & a,const BigInt & b)
	{
		if (a.getNumBytes() == b.getNumBytes())
		{
			return memcmp(a.data,b.data,a.getNumBytes()) == 0;
		}
		else if (a.getNumBytes() > b.getNumBytes())
		{
		//	a.hexDump();
		//	b.hexDump();
			Uint32 ci = a.getNumBytes() - b.getNumBytes();
			for (Uint32 i = 0;i < ci;i++)
				if (a.data[i] > 0)
					return false; 
			
			return memcmp(a.data+ci,b.data,b.getNumBytes()) == 0;
		}
		else
		{
			Uint32 ci = b.getNumBytes() - a.getNumBytes();
			for (Uint32 i = 0;i < ci;i++)
				if (b.data[i] > 0)
					return false; 
			
			return memcmp(a.data,b.data+ci,a.getNumBytes()) == 0;
		}
	}

	bool operator != (const BigInt & a,const BigInt & b)
	{
		return !operator == (a,b);
	}
}
