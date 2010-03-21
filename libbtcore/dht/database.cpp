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
#include "database.h"
#include <arpa/inet.h>
#include <k3socketaddress.h>
#include <util/functions.h>
#include <util/log.h>
#include <torrent/globals.h>

using namespace bt;

namespace dht
{
	DBItem::DBItem()
	{
		time_stamp = bt::CurrentTime();
	}
	
	DBItem::DBItem(const KNetwork::KInetSocketAddress & addr) : addr(addr)
	{
		time_stamp = bt::CurrentTime();
	}
	
	DBItem::DBItem(const DBItem & it)
	{
		addr = it.addr;
		time_stamp = it.time_stamp;	
	}
	
	DBItem::~DBItem()
	{}
		
	bool DBItem::expired(bt::TimeStamp now) const
	{
		return (now - time_stamp >= MAX_ITEM_AGE);
	}
	
	DBItem & DBItem::operator = (const DBItem & it)
	{
		addr = it.addr;
		time_stamp = it.time_stamp;	
		return *this;
	}

	Uint32 DBItem::pack(Uint8* buf) const
	{
		if (addr.ipVersion() == 4)
		{
			memcpy(buf,addr.ipAddress().addr(),4);
			WriteUint16(buf,4,addr.port());
			return 6;
		}
		else
		{
			memcpy(buf,addr.ipAddress().addr(),16);
			WriteUint16(buf,16,addr.port());
			return 18;
		}
	}
	
	///////////////////////////////////////////////
	
	Database::Database()
	{
		items.setAutoDelete(true);
	}


	Database::~Database()
	{}

	void Database::store(const dht::Key & key,const DBItem & dbi)
	{
		DBItemList* dbl = items.find(key);
		if (!dbl)
		{
			dbl = new DBItemList();
			items.insert(key,dbl);
		}
		dbl->append(dbi);
	}
	
	void Database::sample(const dht::Key & key,DBItemList & tdbl,bt::Uint32 max_entries)
	{
		DBItemList* dbl = items.find(key);
		if (!dbl)
			return;
		
		if (dbl->count() < (int) max_entries)
		{
			DBItemList::iterator i = dbl->begin();
			while (i != dbl->end())
			{
				tdbl.append(*i);
				i++;
			}
		}
		else
		{
			Uint32 num_added = 0;
			DBItemList::iterator i = dbl->begin();
			while (i != dbl->end() && num_added < max_entries)
			{
				tdbl.append(*i);
				num_added++;
				i++;
			}
		}
	}
	
	void Database::expire(bt::TimeStamp now)
	{
		bt::PtrMap<dht::Key,DBItemList>::iterator itr = items.begin();
		while (itr != items.end())
		{
			DBItemList* dbl = itr->second;
			// newer keys are inserted at the back
			// so we can stop when we hit the first key which is not expired
			while (dbl->count() > 0 && dbl->first().expired(now))
			{
				dbl->pop_front();
			}
			itr++;
		}
	}
	
	dht::Key Database::genToken(const KNetwork::KInetSocketAddress & addr)
	{
		if (addr.ipVersion() == 4)
		{
			Uint8 tdata[14];
			TimeStamp now = bt::CurrentTime();
			// generate a hash of the ip port and the current time
			// should prevent anybody from crapping things up
			bt::WriteUint32(tdata,0,ntohl(addr.ipAddress().IPv4Addr()));
			bt::WriteUint16(tdata,4,addr.port());
			bt::WriteUint64(tdata,6,now);
				
			dht::Key token = SHA1Hash::generate(tdata,14);
			// keep track of the token, tokens will expire after a while
			tokens.insert(token,now);
			return token;
		}
		else
		{
			Uint8 tdata[26];
			TimeStamp now = bt::CurrentTime();
			// generate a hash of the ip port and the current time
			// should prevent anybody from crapping things up
			memcpy(tdata,addr.ipAddress().addr(),16);
			bt::WriteUint16(tdata,16,addr.port());
			bt::WriteUint64(tdata,18,now);
				
			dht::Key token = SHA1Hash::generate(tdata,26);
			// keep track of the token, tokens will expire after a while
			tokens.insert(token,now);
			return token;
		}
	}
	
	bool Database::checkToken(const dht::Key & token,const KNetwork::KInetSocketAddress & addr)
	{
		// the token must be in the map
		if (!tokens.contains(token))
		{
			Out(SYS_DHT|LOG_DEBUG) << "Unknown token" << endl;
			return false;
		}
		
		// in the map so now get the timestamp and regenerate the token
		// using the IP and port of the sender
		TimeStamp ts = tokens[token];
		
		if (addr.ipVersion() == 4)
		{
			Uint8 tdata[14];
			bt::WriteUint32(tdata,0,ntohl(addr.ipAddress().IPv4Addr()));
			bt::WriteUint16(tdata,4,addr.port());
			bt::WriteUint64(tdata,6,ts);
			dht::Key ct = SHA1Hash::generate(tdata,14);
		
			// compare the generated token to the one received
			if (token != ct)  // not good, this peer didn't went through the proper channels
			{
				Out(SYS_DHT|LOG_DEBUG) << "Invalid token" << endl;
				return false;
			}
		}
		else
		{
			Uint8 tdata[26];
		
			memcpy(tdata,addr.ipAddress().addr(),16);
			bt::WriteUint16(tdata,16,addr.port());
			bt::WriteUint64(tdata,18,ts);
				
			dht::Key ct = SHA1Hash::generate(tdata,26);
			// compare the generated token to the one received
			if (token != ct)  // not good, this peer didn't went through the proper channels
			{
				Out(SYS_DHT|LOG_DEBUG) << "Invalid token" << endl;
				return false;
			}
		}
		
		// expire the token
		tokens.remove(token);
		return true;
	}
	
	bool Database::contains(const dht::Key & key) const
	{
		return items.find(key) != 0;
	}
	
	void Database::insert(const dht::Key & key)
	{
		DBItemList* dbl = items.find(key);
		if (!dbl)
		{
			dbl = new DBItemList();
			items.insert(key,dbl);
		}
	}
}
