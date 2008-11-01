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
#include <string.h>
#include <arpa/inet.h>
#include "sha1hashgen.h"
#include "functions.h"



namespace bt
{
	static inline Uint32 LeftRotate(Uint32 x,Uint32 n)
	{
		return ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)));
	}
	
	

	SHA1HashGen::SHA1HashGen() : tmp_len(0),total_len(0)
	{
		
	}


	SHA1HashGen::~SHA1HashGen()
	{}

	SHA1Hash SHA1HashGen::generate(const Uint8* data,Uint32 len)
	{
		h0 = 0x67452301;
		h1 = 0xEFCDAB89;
		h2 = 0x98BADCFE;
		h3 = 0x10325476;
		h4 = 0xC3D2E1F0;
		
		Uint32 num_64_byte_chunks = len / 64;
		Uint32 left_over = len % 64; 
		// proces regular data
		for (Uint32 i = 0;i < num_64_byte_chunks;i++)
		{
			processChunk(data + (64*i));
		}
		
		// calculate the low and high byte of the data length
		Uint32 total[2] = {0,0};
		total[0] += len;
		total[0] &= 0xFFFFFFFF;
		
		if (total[0] < len)
			total[1]++;
		
		Uint32 high = ( total[0] >> 29 ) | ( total[1] <<  3 );
		Uint32 low  = ( total[0] <<  3 );
		
		if (left_over == 0)
		{
			tmp[0] = 0x80;
			for (Uint32 i = 1;i < 56;i++)
				tmp[i] = 0;
			 
			// put in the length as 64-bit integer (BIG-ENDIAN)
			WriteUint32(tmp,56,high);
			WriteUint32(tmp,60,low);
			// process the padding
			processChunk(tmp);
		}
		else if (left_over < 56)
		{
			Uint32 off = num_64_byte_chunks * 64;
			// copy left over bytes in tmp
			memcpy(tmp,data + off, left_over);
			tmp[left_over] = 0x80;
			for (Uint32 i = left_over + 1;i < 56;i++)
				tmp[i] = 0;
			
			// put in the length as 64-bit integer (BIG-ENDIAN)
			WriteUint32(tmp,56,high);
			WriteUint32(tmp,60,low);
			// process the padding
			processChunk(tmp);
		}
		else
		{
			// now we need to process 2 chunks
			Uint32 off = num_64_byte_chunks * 64;
			// copy left over bytes in tmp
			memcpy(tmp,data + off, left_over);
			tmp[left_over] = 0x80;
			for (Uint32 i = left_over + 1;i < 64;i++)
				tmp[i] = 0;
			
			// process first chunk
			processChunk(tmp);
			
			for (Uint32 i = 0;i < 56;i++)
				tmp[i] = 0;
			
			// put in the length as 64-bit integer (BIG-ENDIAN)
			WriteUint32(tmp,56,high);
			WriteUint32(tmp,60,low);
			// process the second chunk
			processChunk(tmp);
		}
				
		// construct final message
		Uint8 hash[20];
		WriteUint32(hash,0,h0);
		WriteUint32(hash,4,h1);
		WriteUint32(hash,8,h2);
		WriteUint32(hash,12,h3);
		WriteUint32(hash,16,h4);
		
		return SHA1Hash(hash);
	}
	
	

	void SHA1HashGen::processChunk(const Uint8* chunk)
	{
		Uint32 w[80];
		for (int i = 0;i < 80;i++)
		{
			if (i < 16)
			{
			//	w[i] = ntohl(*(const Uint32*)(chunk + (4*i))); <- crashes on sparc
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
	
	
	void SHA1HashGen::start()
	{
		h0 = 0x67452301;
		h1 = 0xEFCDAB89;
		h2 = 0x98BADCFE;
		h3 = 0x10325476;
		h4 = 0xC3D2E1F0;
		tmp_len = total_len = 0;
		memset(tmp,0,64);
	}
		
	void SHA1HashGen::update(const Uint8* data,Uint32 len)
	{
		if (tmp_len == 0)
		{
			Uint32 num_64_byte_chunks = len / 64;
			Uint32 left_over = len % 64; 
			// proces data in chunks of 64 byte
			for (Uint32 i = 0;i < num_64_byte_chunks;i++)
			{
				processChunk(data + (64*i));
			}
			 
			if (left_over > 0)
			{
				// if there is anything left over, copy it in tmp
				memcpy(tmp,data + (64 * num_64_byte_chunks),left_over);
				tmp_len = left_over;
			}
			total_len += len;
		}
		else
		{
			
			if (tmp_len + len < 64)
			{
				// special case, not enough of data to fill tmp completely
				memcpy(tmp + tmp_len,data,len);
				tmp_len += len;
				total_len += len;
			}
			else
			{
				// copy start of data in tmp and process it
				Uint32 off = 64 - tmp_len;
				memcpy(tmp + tmp_len,data, 64 - tmp_len);
				processChunk(tmp); 
				tmp_len = 0;
				
				Uint32 num_64_byte_chunks = (len - off) / 64;
				Uint32 left_over = (len - off) % 64;
				
				for (Uint32 i = 0;i < num_64_byte_chunks;i++)
				{
					processChunk(data + (off + (64*i)));
				}
				
				if (left_over > 0)
				{
				// if there is anything left over, copy it in tmp
					memcpy(tmp,data + (off + 64 * num_64_byte_chunks),left_over);
					tmp_len = left_over;
				}
				total_len += len;
			}
		}
	}
		
	 
	void SHA1HashGen::end()
	{
		// calculate the low and high byte of the data length
		Uint32 total[2] = {0,0};
		total[0] += total_len;
		total[0] &= 0xFFFFFFFF;
		
		if (total[0] < total_len)
			total[1]++;
		
		Uint32 high = ( total[0] >> 29 ) | ( total[1] <<  3 );
		Uint32 low  = ( total[0] <<  3 );
		
		if (tmp_len == 0)
		{
			tmp[0] = 0x80;
			for (Uint32 i = 1;i < 56;i++)
				tmp[i] = 0;
			 
			// put in the length as 64-bit integer (BIG-ENDIAN)
			WriteUint32(tmp,56,high);
			WriteUint32(tmp,60,low);
			// process the padding
			processChunk(tmp);
		}
		else if (tmp_len < 56)
		{
			tmp[tmp_len] = 0x80;
			for (Uint32 i = tmp_len + 1;i < 56;i++)
				tmp[i] = 0;
			
			// put in the length as 64-bit integer (BIG-ENDIAN)
			WriteUint32(tmp,56,high);
			WriteUint32(tmp,60,low);
			// process the padding
			processChunk(tmp);
		}
		else
		{
			// now we need to process 2 chunks
			tmp[tmp_len] = 0x80;
			for (Uint32 i = tmp_len + 1;i < 56;i++)
				tmp[i] = 0;
			
			// process first chunk
			processChunk(tmp);
			
			for (Uint32 i = 0;i < 56;i++)
				tmp[i] = 0;
			
			// put in the length as 64-bit integer (BIG-ENDIAN)
			WriteUint32(tmp,56,high);
			WriteUint32(tmp,60,low);
			// process the second chunk
			processChunk(tmp);
		}
	}
		

	SHA1Hash SHA1HashGen::get() const
	{
		// construct final message
		Uint8 hash[20];
		WriteUint32(hash,0,h0);
		WriteUint32(hash,4,h1);
		WriteUint32(hash,8,h2);
		WriteUint32(hash,12,h3);
		WriteUint32(hash,16,h4);
		
		return SHA1Hash(hash);
	}
}
