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
	Uint32 AntiP2P::toUint32(QString& ip)
	{
		bool test;
		Uint32 ret = ip.section('.',0,0).toULongLong(&test);
		ret <<= 8;
		ret |= ip.section('.',1,1).toULong(&test);
		ret <<= 8;
		ret |= ip.section('.',2,2).toULong(&test);
		ret <<= 8;
		ret |= ip.section('.',3,3).toULong(&test);

		return ret;
	}
	
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
		uint blocksize = nrElements < 100 ? 10 : 100; // number of entries that each HeaderBlock holds. If total number is < 100, than this value is 10.
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
		header_loaded = true;
	}
	
	bool AntiP2P::exists()
	{
		return file != 0;
	}
	
	bool AntiP2P::isBlockedIP( QString& ip )
	{
		Uint32 test = toUint32(ip);
		return isBlockedIP(test);
	}
	
	int AntiP2P::searchHeader(Uint32& ip, int start, int end)
	{
		if (end == 0)
			return -1; //empty list
		
		if (end == 1)
		{
			if (blocks[start].ip1 <= ip && blocks[start].ip2 >= ip) //then our IP is somewhere in between
			{
				if (blocks[start].ip1 == ip || blocks[start].ip2 == ip)
					return -2; //Return -2 to signal that this IP matches either IP from header. No need to search mmaped file in that case.
				else
					return start; //else return block index
			}
			else
				return -1; //not found
		}
		else
		{
			int i = start + end/2;
			if (blocks[i].ip1 <= ip)
				return searchHeader(ip, i, end - end/2);
			else
				return searchHeader(ip, start, end/2);
		}
	}
	
	bool AntiP2P::isBlockedIP( Uint32& ip )
	{
		if (!header_loaded)
		{
			Out() << "Tried to check if IP was blocked, but no AntiP2P header was loaded." << endl;
			return false;
		}

		int in_header = searchHeader(ip, 0, blocks.count());
		switch (in_header)
		{
			case -1:
				return false; //ip is not blocked
			case -2:
				return true;  //ip is blocked (we're really lucky to find it in header already)
			default:
				//search mmapped file
				HeaderBlock to_be_searched = blocks[in_header];
				Uint8* fptr = (Uint8*) file->getDataPointer();
				fptr += to_be_searched.offset;
				IPBlock* file_blocks =  (IPBlock*) fptr;
				return searchFile(file_blocks, ip, 0, to_be_searched.nrEntries);
				break;
		}
		return false;
	}
	
	bool AntiP2P::searchFile(IPBlock* file_blocks, Uint32& ip, int start, int end)
	{
		if (end == 0)
			return -1; //empty list
		
		if (end == 1)
		{
			if (file_blocks[start].ip1 <= ip && file_blocks[start].ip2 >= ip) //we have a match!
				return true;
			else
				return false; //IP is not found.
		}
		
		else
		{
			int i = start + end/2;
			if (file_blocks[i].ip1 <= ip)
				return searchFile(file_blocks, ip, i, end - end/2);
			else
				return searchFile(file_blocks, ip, start, end/2);
		}
	}
}
