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
#include <iostream>
#include <stdlib.h>
#include <util/log.h>
#include <util/error.h>
#include <util/functions.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>
#include "singlecachechecker.h"
#include "multicachechecker.h"


using namespace bt;
using namespace ktdebug;

void Help()
{
	Out() << "Usage : cachecheck <directory_containing_cache>" << endl;
	Out() << "OR cachecheck <torrent> <cache_file_or_dir> <index_file>" << endl;
	exit(0);
}

int main(int argc,char** argv)
{
	Globals::instance().setDebugMode(true);
	Globals::instance().initLog("cachecheck.log");
	CacheChecker* cc = 0;
	try
	{
		Torrent tor;
		QString tor_file,cache,index;

		if (argc == 2)
		{
			QString cache_dir = argv[1];
			if (!cache_dir.endsWith(bt::DirSeparator()))
				cache_dir += bt::DirSeparator();

			tor_file = cache_dir + "torrent";
			cache = cache_dir + "cache";
			index = cache_dir + "index";
		}
		else if (argc == 4)
		{
			tor_file = argv[1];
			cache = argv[2];
			index = argv[3];
		}
		else
		{
			Help();
		}

		
		Out() << "Loading torrent : " << tor_file << " ... " << endl;
		tor.load(tor_file,false);
		if (tor.isMultiFile())
			cc = new MultiCacheChecker(tor);
		else
			cc = new SingleCacheChecker(tor);

		cc->check(cache,index);
		if (cc->foundFailedChunks())
		{
			std::string str;
			std::cout << "Found failed chunks, fix index file ? (y/n) ";
			std::cin >> str;
			if (str == "y" || str == "Y")
			{
				Out() << "Fixing index file ..." << endl;
				cc->fixIndex();
				Out() << "Finished !" << endl;
			}
		}
	}
	catch (Error & e)
	{
		Out() << "Error : " << e.toString() << endl;
	}
	
	delete cc;
	Globals::cleanup();
	return 0;
}
