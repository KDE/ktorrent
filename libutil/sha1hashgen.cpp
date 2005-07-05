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
#include <string.h>
#include "sha1hashgen.h"
#include "functions.h"



namespace bt
{
	static inline Uint32 LeftRotate(Uint32 x,Uint32 n)
	{
		return ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)));
	}
	
	

	SHA1HashGen::SHA1HashGen()
	{
		
	}


	SHA1HashGen::~SHA1HashGen()
	{}

	SHA1Hash SHA1HashGen::generate(Uint8* data,Uint32 len)
	{
		h0 = 0x67452301;
		h1 = 0xEFCDAB89;
		h2 = 0x98BADCFE;
		h3 = 0x10325476;
		h4 = 0xC3D2E1F0;
		
		// first preprocess input : 
		// calculate new length
		Uint32 input_len = len;
		while (input_len % 64 != 55)
			input_len++;;
		input_len += 9;
		
		// allocate and copy over data
		Uint8* input = new Uint8[input_len];
		memcpy(input,data,len);
		
		// fill in the padding
		input[len] = 0x80;
		for (Uint32 i = len+1;i % 64 != 56;i++)
			input[i] = 0x00;
		
		
		Uint32 total[2] = {0,0};
		total[0] += len;
		total[0] &= 0xFFFFFFFF;
		
		if (total[0] < len)
			total[1]++;
		
		Uint32 high = ( total[0] >> 29 ) | ( total[1] <<  3 );
		Uint32 low  = ( total[0] <<  3 );

		
		// put in the length as 64-bit integer (BIG-ENDIAN)
		WriteUint32(input,input_len-8,high);
		WriteUint32(input,input_len-4,low);
		
		// process chunks of 64 byte each
		for (Uint32 i = 0;i < input_len;i+=64)
		{
			processChunk(input+i);
		}
		// construct final message
		Uint8 hash[20];
		WriteUint32(hash,0,h0);
		WriteUint32(hash,4,h1);
		WriteUint32(hash,8,h2);
		WriteUint32(hash,12,h3);
		WriteUint32(hash,16,h4);
		delete [] input;
		return SHA1Hash(hash);
	}
	
	

	void SHA1HashGen::processChunk(Uint8* chunk)
	{
		Uint32 w[80];
		for (int i = 0;i < 80;i++)
		{
			if (i < 16)
			{
				w[i] = (chunk[4*i] << 24) | 
						(chunk[4*i + 1] << 16) | 
						(chunk[4*i + 2] << 8) | 
						chunk[4*i + 3];
			}
			else
			{
				w[i] = LeftRotate(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16],1);
			}
		}
		
		Uint32 a = h0;
		Uint32 b = h1;
		Uint32 c = h2;
		Uint32 d = h3;
		Uint32 e = h4;
		
		for (int i = 0;i < 80;i++)
		{
			Uint32 f,k;
			if (i < 20)
			{
				f = (b & c) | ((~b) & d);
				k = 0x5A827999;
			}
			else if (i < 40)
			{
				f = b ^ c ^ d;
				k = 0x6ED9EBA1;
			}
			else if (i < 60)
			{
				f = (b & c) | (b & d) | (c & d);
				k = 0x8F1BBCDC;
			}
			else
			{
				f = b ^ c ^ d;
				k = 0xCA62C1D6;
			}

			Uint32 temp = LeftRotate(a,5) + f + e + k + w[i];
			e = d;
			d = c;
			c = LeftRotate(b,30);
			b = a;
			a = temp;
		}
		h0 = (h0 + a) & 0xffffffff;
		h1 = (h1 + b) & 0xffffffff;
		h2 = (h2 + c) & 0xffffffff;
		h3 = (h3 + d) & 0xffffffff;
		h4 = (h4 + e) & 0xffffffff;
	}
}
