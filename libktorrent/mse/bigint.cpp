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
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <util/log.h>
#include <util/functions.h>
#include <torrent/globals.h>
#include "bigint.h"

using namespace bt;

namespace mse
{


	BigInt::BigInt(Uint32 num_bits)
	{
		mpz_init2(val,num_bits);	
	}
		
	BigInt::BigInt(const QString & value)
	{
		mpz_init2(val,(value.length() - 2)*4);
		mpz_set_str(val,value.ascii(),0);	
	}
		
	BigInt::BigInt(const BigInt & bi)
	{
		mpz_set(val,bi.val);
	}
	
	BigInt::~BigInt()
	{
		mpz_clear(val);
	}
	
	
	BigInt & BigInt::operator = (const BigInt & bi)
	{
		mpz_set(val,bi.val);
		return *this;
	}
	
	BigInt BigInt::powerMod(const BigInt & x,const BigInt & e,const BigInt & d)
	{
		BigInt r;
		mpz_powm(r.val,x.val,e.val,d.val);
		return r;
	}
	
	BigInt BigInt::random()
	{
		static Uint32 rnd = 0;
		if (rnd % 10 == 0)
		{
			TimeStamp now = bt::GetCurrentTime();
			srand(now);
			rnd = 0;
		}
		rnd++;
		Uint8 tmp[20];
		for (Uint32 i = 0;i < 20;i++)
			tmp[i] = (Uint8)rand() % 0x100;
		
		return BigInt::fromBuffer(tmp,20);
	}

	Uint32 BigInt::toBuffer(Uint8* buf,Uint32 max_size) const
	{
		size_t foo;
		mpz_export(buf,&foo,1,1,1,0,val);
		return foo;
	}
	
	BigInt BigInt::fromBuffer(const Uint8* buf,Uint32 size)
	{
		BigInt r(size*8);
		mpz_import(r.val,size,1,1,1,0,buf);
		return r;
	}
	
}
