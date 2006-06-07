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
#include <mse/functions.h>
#include <util/log.h>
#include <torrent/globals.h>
#include "difflehellmantest.h"

using namespace bt;
using namespace mse;

namespace utest
{

	DiffleHellmanTest::DiffleHellmanTest()
			: UnitTest("DiffleHellman")
	{}


	DiffleHellmanTest::~DiffleHellmanTest()
	{}


	bool DiffleHellmanTest::doTest()
	{
		BigInt xa,ya,xb,yb;
		mse::GeneratePublicPrivateKey(xa,ya);
		mse::GeneratePublicPrivateKey(xb,yb);
		mse::DumpBigInt("Xa",xa);
		mse::DumpBigInt("Ya",ya);
		mse::DumpBigInt("Xb",xb);
		mse::DumpBigInt("Yb",yb);
		
		mse::DumpBigInt("Sa",mse::DHSecret(xa,yb));
		mse::DumpBigInt("Sb",mse::DHSecret(xb,ya));
		return true;
	}

}
