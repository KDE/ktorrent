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
	
	RC4::RC4(const Uint8* key,Uint32 size) : i(0),j(0)
	{
		// initialize state
		for (Uint32 t = 0;t < 256;t++)
			s[t] = t;
		
		j = 0;
		for (Uint32 t=0;t < 256;t++)
		{
			j = (j + s[t] + key[t % size]) & 0xff;
			swap(s[t],s[j]);
		}
		
		i = j = 0;
	}
	
	RC4::~RC4()
	{
	}
		
	void RC4::process(const Uint8* in,Uint8* out,Uint32 size)
	{
		for (Uint32 k = 0;k < size;k++)
		{
			out[k] = process(in[k]);
		}
	}
	
	Uint8 RC4::process(Uint8 b)
	{
		i = (i + 1) & 0xff;
		j = (j + s[i]) & 0xff;
		swap(s[i],s[j]);
		Uint8 tmp = s[ (s[i] + s[j]) & 0xff];
		return tmp ^ b;
	}
	

	RC4Encryptor::RC4Encryptor(const bt::SHA1Hash & dk,const bt::SHA1Hash & ek) 
	: enc(ek.getData(),20),dec(dk.getData(),20)
	{
		Uint8 tmp[1024];
		enc.process(tmp,tmp,1024);
		dec.process(tmp,tmp,1024);
	}


	RC4Encryptor::~RC4Encryptor()
	{}


	void RC4Encryptor::decrypt(Uint8* data,Uint32 len)
	{
		dec.process(data,data,len);
	}

	const Uint8* RC4Encryptor::encrypt(const Uint8* data,Uint32 len)
	{
		enc.process(data,rc4_enc_buffer,len);
		return rc4_enc_buffer;
	}
	
	void RC4Encryptor::encryptReplace(Uint8* data,Uint32 len)
	{
		enc.process(data,data,len);
	}
	
}
