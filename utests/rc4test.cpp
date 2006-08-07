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
#include <stdio.h>
#include <mse/rc4encryptor.h>
#include <util/log.h>
#include <torrent/globals.h>
#include "rc4test.h"

using namespace bt;
using namespace mse;

namespace utest
{

	RC4Test::RC4Test() : UnitTest("RC4")
	{}


	RC4Test::~RC4Test()
	{}


	bool RC4Test::doTest()
	{
		bool ret1 = firstTest();
		bool ret2 = secondTest();
		return ret1 && ret2;
	}
	
	bool RC4Test::firstTest()
	{
		Out() << "First RC4 test" << endl;
		SHA1Hash a = SHA1Hash::generate((Uint8*)"keyA",4);
		SHA1Hash b = SHA1Hash::generate((Uint8*)"keyB",4);
		
		RC4Encryptor as(b,a);
		RC4Encryptor bs(a,b);
		char* test = "Dit is een test";
		int tlen = strlen(test);
		Uint8* dec = (Uint8*)as.encrypt((Uint8*)test,tlen);
		bs.decrypt(dec,tlen);
		if (memcmp(dec,test,tlen) == 0)
		{
			Out() << "Test succesfull" << endl;
			Out() << QString(test) << endl;
			Out() << QString((char*)dec) << endl;
			return true;
		}
		else
		{
			Out() << "Test not succesfull" << endl;
			Out() << QString(test) << endl;
			Out() << QString((char*)dec) << endl;
			return false;
		}
	}
	
	bool RC4Test::secondTest()
	{
		Out() << "Second RC4 test" << endl;
		Uint8 output[100];
		Uint8 result[] = {0xbb,0xf3,0x16, 0xe8 , 0xd9, 0x40, 0xaf,0x0a ,0xd3 };
		// RC4( "Key", "Plaintext" ) == "bbf316e8 d940af0a d3"
		char* key = "Key";
		char* pt = "Plaintext";
		RC4 enc((const Uint8*)key,3);
		enc.process((const Uint8*)pt,output,strlen(pt));
		
		for (Uint32 i = 0;i < strlen(pt);i++)
		{
			if (output[i] != result[i])
				return false;
		}
		
		return true;
	}

}
