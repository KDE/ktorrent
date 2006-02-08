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
#include "kbucket.h"

namespace dht
{
	KBucketEntry::KBucketEntry()
	{
	}
	
	KBucketEntry::KBucketEntry(const KInetSocketAddress & addr,const Key & id)
	: addr(addr),node_id(id)
	{
	}
		
	KBucketEntry::KBucketEntry(const KBucketEntry & other)
	: addr(other.addr),node_id(other.node_id)
	{}

		
	KBucketEntry::~KBucketEntry()
	{}

	KBucketEntry & KBucketEntry::operator = (const KBucketEntry & other)
	{
		addr = other.addr;
		node_id = other.node_id;
		return *this;
	}
	
	bool KBucketEntry::operator == (const KBucketEntry & entry) const
	{
		return addr == entry.addr && node_id == entry.node_id;
	}

	KBucket::KBucket()
	{}
	
	
	KBucket::~KBucket()
	{}
	
	bool KBucket::insert(const KBucketEntry & entry,bool force)
	{
		QValueList<KBucketEntry>::iterator i = entries.find(entry);
	
		// If in the list, move it to the end
		if (i != entries.end())
		{
			entries.remove(i);
			entries.append(entry);
			return true;
		}
		
		// insert if not allready in the list and we still have room
		if (i == entries.end() && entries.count() < dht::K)
		{
			entries.append(entry);
			return true;
		}
		
		// if force is on, get rid of the first and append the entry
		if (force)
		{
			entries.pop_front();
			entries.append(entry);
			return true;
		}
		
		return false;
	}

	bool KBucket::contains(const KBucketEntry & entry) const
	{
		return entries.contains(entry);
	}
}

