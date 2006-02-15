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
#include <util/functions.h>
#include "kclosestnodessearch.h"

using namespace bt;
using namespace KNetwork;

namespace dht
{
	typedef std::map<dht::Key,KBucketEntry>::iterator KNSitr;

	KClosestNodesSearch::KClosestNodesSearch(const dht::Key & key,Uint32 max_entries) 
	: key(key),max_entries(max_entries)
	{}


	KClosestNodesSearch::~KClosestNodesSearch()
	{}

	
	void KClosestNodesSearch::tryInsert(const KBucketEntry & e)
	{
		// calculate distance between key and e
		dht::Key d = dht::Key::distance(key,e.getID());
		
		if (emap.size() < max_entries)
		{
			// room in the map so just insert
			emap.insert(std::make_pair(d,e));
		}
		else
		{
			// now find the max distance
			// seeing that the last element of the map has also 
			// the biggest distance to key (std::map is sorted on the distance)
			// we just take the last
			const dht::Key & max = emap.rbegin()->first;
			if (d < max)
			{
				// insert if d is smaller then max
				emap.insert(std::make_pair(d,e));
				// erase the old max value
				emap.erase(max);
			}
		}
		
	}
	
	void KClosestNodesSearch::pack(Uint8* buffer,Uint32 max_size)
	{
		// make sure we do not writ to much
		Uint32 max_items = max_size / 26;
		Uint32 j = 0;
		Uint8* ptr = buffer; // ptr we move around in the buffer
		
		
		KNSitr i = emap.begin();
		while (i != emap.end() && j < max_items)
		{
			KBucketEntry & e = i->second;
			const KInetSocketAddress & addr = e.getAddress();
			// copy ID, IP address and port into the buffer
			memcpy(ptr,e.getID().getData(),20);
			bt::WriteUint32(ptr,20,addr.ipAddress().IPv4Addr());
			bt::WriteUint16(ptr,24,addr.port());
			
			// updata pointer and j counter
			ptr = ptr + 26;
			j++;
		}
	}

}
