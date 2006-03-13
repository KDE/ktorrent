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
#include <stdlib.h>
#include <libktorrent/util/log.h>
#include <libktorrent/torrent/globals.h>
#include <plugins/upnp/upnprouter.h>
#include <plugins/upnp/upnpdescriptionparser.h>

using namespace kt;
using namespace bt;

void help()
{
	Out() << "Usage : ktupnptest <textfile>" << endl;
	exit(0);
}

int main(int argc,char** argv)
{
	Globals::instance().setDebugMode(true);
	Globals::instance().initLog("ktupnptest.log");
	if (argc < 2)
		help();
	
	
	kt::UPnPRouter router(QString::null,"http://foobar.com");
	kt::UPnPDescriptionParser dp;
	
	if (!dp.parse(argv[1],&router))
	{
		Out() << "Cannot parse " << QString(argv[1]) << endl;
	}
	else
	{
		Out() << "Succesfully parsed the UPnP contents" << endl;
		router.debugPrintData();
	}
	
	Globals::cleanup();
	return 0;
}
