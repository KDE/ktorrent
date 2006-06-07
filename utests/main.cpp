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
#include <stdlib.h>
#include <libktorrent/util/log.h>
#include <libktorrent/torrent/globals.h>
#include "testrunner.h"
#include "upnpparsedescriptiontest.h"
#include "upnpparseresponsetest.h"
#include "dhtmsgparsetest.h"
#include "biginttest.h"
#include "rc4test.h"
#include "difflehellmantest.h"

using namespace kt;
using namespace bt;
using namespace utest;



int main(int argc,char** argv)
{
	Globals::instance().setDebugMode(true);
	Globals::instance().initLog("ktutester.log");
	TestRunner tr;
	tr.addTest(new UPnPParseDescriptionTest());
	tr.addTest(new UPnPParseResponseTest());
	tr.addTest(new DHTMsgParseTest());
	tr.addTest(new BigIntTest());
	tr.addTest(new RC4Test());
	tr.addTest(new DiffleHellmanTest());
	tr.doAllTests();
	Globals::cleanup();
	return 0;
}
