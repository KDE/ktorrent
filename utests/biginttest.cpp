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
#include <mse/bigint.h>
#include <util/log.h>
#include <torrent/globals.h>
#include "biginttest.h"

using namespace bt;
using namespace mse;

namespace utest
{

	BigIntTest::BigIntTest() : UnitTest("BigIntTest")
	{}


	BigIntTest::~BigIntTest()
	{}
	
	static void PrintBigInt(BigInt & b)
	{
		Uint8 buf[10];
		memset(buf,0,10);
		b.toBuffer(buf,10);
		for (Uint32 i = 0;i < 10;i++)
		{
			Out() << QString("0x%1 ").arg(buf[i],0,16);
		}
		Out() << endl;
	}

	bool BigIntTest::doTest()
	{
		Out() << "First test : " << endl;
		BigInt a("0x1E");
		BigInt b("0x42");
		BigInt c("0xFFFFEE");
		BigInt d = BigInt::powerMod(a,b,c);
		PrintBigInt(a);
		PrintBigInt(b);
		PrintBigInt(c);
		PrintBigInt(d);
		Out() << "Second test : " << endl;
		Uint8 test[] = {0xAB,0x12,0x34,0xE4,0xF6};
		a = BigInt::fromBuffer(test,5);
		PrintBigInt(a);
		Uint8 foobar[5];
		a.toBuffer(foobar,5);
		for (Uint32 i = 0;i < 5;i++)
		{
			Out() << QString("0x%1 ").arg(foobar[i],0,16);
		}
		Out() << endl;
		Out() << "Third test" << endl;
		a = BigInt("0xABCD1234");
		PrintBigInt(a);
		a.toBuffer(foobar,4);
		c = BigInt::fromBuffer(foobar,4);
		PrintBigInt(c);
		return true;
	}

}
