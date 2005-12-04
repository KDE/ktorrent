/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include "antip2p.h"

#include <torrent/globals.h>
#include <util/log.h>
#include <util/constants.h>
#include <util/mmapfile.h>

#include <kglobal.h>
#include <kstandarddirs.h>

#include <qstring.h>
#include <qvaluelist.h>

using namespace bt;

namespace kt
{
	typedef struct
	{
		Uint32 ip1;
		Uint32 ip2;
	} IPBlock;
	
	AntiP2P::AntiP2P()
	{
		header_loaded = false;
		
		file = new MMapFile();
		if(! file->open(KGlobal::dirs()->saveLocation("data","ktorrent") + "level1.dat", MMapFile::READ) )
		{
			Out() << "Anti-p2p file not loaded." << endl;
			file = 0;
			return;
		}
  	}

  	AntiP2P::~AntiP2P()
  	{
		if(file)
			delete file;
	}
	
	void AntiP2P::loadHeader()
	{
		if(!file)
			return;
		
		Uint32 nrElements = file->getSize() / sizeof(IPBlock);
		uint blocksize = 10000; //nrElements < 100 ? 10 : 100; // number of entries that each HeaderBlock holds. If total number is < 100, than this value is 10.
		HeaderBlock hb;

		for(Uint64 i = 0; i < file->getSize() ; i+= sizeof(IPBlock)*(blocksize) )
		{
			IPBlock ipb;
			hb.offset = i;
			file->seek(MMapFile::BEGIN, i);
			file->read(&ipb, sizeof(IPBlock));
			hb.ip1 = ipb.ip1;
			file->seek(MMapFile::BEGIN, i  + (blocksize-1)*sizeof(IPBlock));
			file->read(&ipb, sizeof(IPBlock));
			hb.ip2 = ipb.ip2;
			hb.nrEntries = blocksize;
			if ( i  + (blocksize-1)*sizeof(IPBlock) > file->getSize() ) //last entry
			{
				hb.nrEntries = file->getSize() % blocksize;
			}
			blocks.push_back(hb);
		}
		
		Out() << "AntiP2P header loaded." << endl;
	}
	
	bool AntiP2P::exists()
	{
		return file == 0;
	}
}
