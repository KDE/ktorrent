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

	static QString BigIntToString(const mse::BigInt & bi)
	{
		QString t;
		bool odd = bi.getNumBits() / 8 != bi.getNumBytes();
		for (Uint32 i = 0;i < bi.getNumBytes();i++)
		{
			Uint8 b = *(bi.getData() + i);
			if (i == 0)
			{
				if (!odd)
				{
					t += QString::number((b & 0xF0) >> 4,16).upper() +
						QString::number((b & 0x0F),16).upper();
				}
				else
				{
					QString tmp;
					if (b & 0xF0 != 0)
						tmp = QString::number((b & 0xF0) >> 4,16).upper();
				
					t += tmp + QString::number((b & 0x0F),16).upper();
				}
			}
			else
			{
				t += QString::number((b & 0xF0) >> 4,16).upper() +
						QString::number((b & 0x0F),16).upper();
			}
		}
		
		Out() << "BigIntToString : 0x" << t << endl;
		return t;
	}
	
	bool BigIntTest::conversionTest()
	{
		QString str = "DEADBEEF1234";
		mse::BigInt bi(str);
			
		if (str != BigIntToString(bi))
		{
			Out() << "Conversion test 1 failure" << endl;
			return false;
		}
			
		str = "1200ABC";
		bi = BigInt(str);
			
		if (0x01200ABC != BigIntToString(bi).toULong(0,16))
		{
			Out() << "Conversion test 2 failure " << BigIntToString(bi) << endl;
			return false;
		}
		
		BigInt d("000ABC");
		if (BigIntToString(d).toULong(0,16) != 0x000ABC)
		{
			Out() << "Conversion test 3 failure" << endl;
			return false;
		}	
		
		Out() << "Conversion test succes" << endl;
		return true;
	}	
	
	bool BigIntTest::xorTest()
	{
		QString a = "1234EE0A";
		QString b = "1700FF23";
		bt::Uint32 ai = 0x1234EE0A;
		bt::Uint32 bi = 0x1700FF23;
	
		Out() << "ai ^ bi = 0x" << QString::number(ai ^ bi,16) << endl;
		
		BigInt amb = BigInt::exclusiveOr(a,b);
		if (BigIntToString(amb).toULong(0,16) != QString::number(ai ^ bi,16).upper().toULong(0,16))
		{
			Out() << "XOR test failure" << endl;
			return false;
		}
		else
		{
			Out() << "XOR test succes" << endl;
			return true;
		}
	}
	
	bool BigIntTest::modTest()
	{
		BigInt b("10A");
		BigInt a("1700FF23");
		bt::Uint32 bi = 0x10A;
		bt::Uint32 ai = 0x1700FF23;
		
		if (BigIntToString(BigInt::mod(a,b)).toULong(0,16) != ai % bi)
		{
			Out() << "Mod test failure" << endl;
			return false;
		}
		else
		{
			Out() << "Mod test succes" << endl;
			return true;
		}
	}
	
	bool BigIntTest::cmpTest()
	{
		BigInt a("123ABC");
		BigInt b("EEF");
		if (a > b != true)
		{
			Out() << "Cmp test 1 failure" << endl;
			return false;
		}
		
		BigInt c;
		if (c > a != false)
		{
			Out() << "Cmp test 2 failure" << endl;
			return false;
		}
		
		if (a > c != true)
		{
			Out() << "Cmp test 3 failure" << endl;
			return false;
		}
		
		BigInt d("000ABC");
		BigInt e("ABC");
		if (d != e)
		{
			Out() << "Cmp test 4 failure" << endl;
			return false;
		}
		
		Out() << "Cmp test success" << endl;
		return true;
	}
	

	bool BigIntTest::doTest()
	{
		bool ret = conversionTest();
		ret = xorTest() && ret; 
		ret = cmpTest() && ret;
		ret = modTest() && ret;
		return ret;
	}

}
