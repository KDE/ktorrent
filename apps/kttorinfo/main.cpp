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
#include <stdlib.h>
#include <qstring.h>
#include <util/log.h>
#include <util/error.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>

using namespace bt;

void help()
{
	Out() << "Usage : kttorinfo <torrent>" << endl;
	exit(0);
}

int main(int argc,char** argv)
{
	Globals::instance().setDebugMode(true);
	Globals::instance().initLog("kttorinfo.log");
	if (argc < 2)
		help();
	
	try
	{
		Torrent tor;
		tor.load(argv[1],false);
		tor.debugPrintInfo();
	}
	catch (Error & e)
	{
		Out() << "Error : " << e.toString() << endl;
	}
	Globals::cleanup();
	return 0;
}
