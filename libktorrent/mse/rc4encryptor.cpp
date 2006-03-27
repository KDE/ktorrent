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
#include "rc4encryptor.h"

namespace mse
{
	static void swap(Uint8 & a,Uint8 & b)
	{
		Uint8 tmp = a;
		a = b;
		b = tmp;
	}
	
	static Uint8 rc4_enc_buffer[bt::MAX_MSGLEN];

	RC4Encryptor::RC4Encryptor(const bt::SHA1Hash & dkey,const bt::SHA1Hash & ekey) 
	: dkey(dkey),ekey(ekey),di(0),dj(0),ei(0),ej(0)
	{
		Uint32 j = 0,i = 0;
		// Key scheduling algorithm for decrypt and encrypt side
		for (i = 0;i < 256;i++)
			ds[i] = i;
		
		j = 0;
		for (i=0;i < 255;i++)
		{
			j = (j + ds[i] + dkey[i % 20]) % 256;
			swap(ds[i],ds[j]);
		}
		
		for (i = 0;i < 256;i++)
			es[i] = i;
		
		j = 0;
		for (i=0;i < 255;i++)
		{
			j = (j + es[i] + ekey[i % 20]) % 256;
			swap(es[i],es[j]);
		}
	}


	RC4Encryptor::~RC4Encryptor()
	{}


	void RC4Encryptor::decrypt(Uint8* data,Uint32 len)
	{
		for (Uint32 k = 0;k < len;k++)
		{
			data[k] = prga(true) ^ data[k];
		}
	}

	const Uint8* RC4Encryptor::encrypt(const Uint8* data,Uint32 len)
	{
		for (Uint32 k = 0;k < len;k++)
		{
			rc4_enc_buffer[k] = prga(false) ^ data[k];
		}
		return rc4_enc_buffer;
	}
	
	Uint8 RC4Encryptor::prga(bool d)
	{
		if (d)
		{
			di = (di + 1) % 256;
			dj = (dj + ds[di]) % 256;
			swap(ds[di],ds[dj]);
			return ds[ (ds[di] + ds[dj]) % 256];
		}
		else
		{
			ei = (ei + 1) % 256;
			ej = (ej + es[ei]) % 256;
			swap(es[ei],es[ej]);
			return es[ (es[ei] + es[ej]) % 256];
		}
	}

}
