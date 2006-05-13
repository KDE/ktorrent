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
#include <ksocketaddress.h>
#include <util/file.h>
#include <util/functions.h>
#include <netinet/in.h>
#include "kbucket.h"
#include "kclosestnodessearch.h"
#include "rpcserver.h"
#include "node.h"

using namespace KNetwork;

namespace dht
{
	KBucketEntry::KBucketEntry()
	{
		last_responded = bt::GetCurrentTime();
		failed_queries = 0;
	}
	
	KBucketEntry::KBucketEntry(const KInetSocketAddress & addr,const Key & id)
	: addr(addr),node_id(id)
	{
		last_responded = bt::GetCurrentTime();
		failed_queries = 0;
	}
		
	KBucketEntry::KBucketEntry(const KBucketEntry & other)
	: addr(other.addr),node_id(other.node_id),
	last_responded(other.last_responded),failed_queries(other.failed_queries)
	{}

		
	KBucketEntry::~KBucketEntry()
	{}

	KBucketEntry & KBucketEntry::operator = (const KBucketEntry & other)
	{
		addr = other.addr;
		node_id = other.node_id;
		last_responded = other.last_responded;
		failed_queries = other.failed_queries;
		return *this;
	}
	
	bool KBucketEntry::operator == (const KBucketEntry & entry) const
	{
		return addr == entry.addr && node_id == entry.node_id;
	}
	
	bool KBucketEntry::isGood() const
	{
		if (bt::GetCurrentTime() - last_responded > 15 * 60 * 1000)
			return false;
		else
			return true;
	}
		
	bool KBucketEntry::isQuestionable() const
	{
		if (bt::GetCurrentTime() - last_responded > 15 * 60 * 1000)
			return true;
		else
			return false;
	}
		
	
	bool KBucketEntry::isBad() const
	{
		if (isGood())
			return false;
		
		return failed_queries > 2;
	}
	
	void KBucketEntry::hasResponded()
	{
		last_responded = bt::GetCurrentTime();
		failed_queries = 0; // reset failed queries
	}
	

	//////////////////////////////////////////////////////////
	
	KBucket::KBucket(RPCServer* srv,Node* node) : srv(srv),node(node)
	{
		last_modified = bt::GetCurrentTime();
	}
	
	
	KBucket::~KBucket()
	{}
	
	void KBucket::insert(const KBucketEntry & entry)
	{
		QValueList<KBucketEntry>::iterator i = entries.find(entry);
	
		// If in the list, move it to the end
		if (i != entries.end())
		{
			KBucketEntry & e = *i;
			e.hasResponded();
			last_modified = bt::GetCurrentTime();
			entries.remove(i);
			entries.append(entry);
			return;
		}
		
		// insert if not allready in the list and we still have room
		if (i == entries.end() && entries.count() < dht::K)
		{
			entries.append(entry);
			last_modified = bt::GetCurrentTime();
		}
		else if (!replaceBadEntry(entry))
		{
			// ping questionable nodes when replacing a bad one fails
			pingQuestionable(entry);	
		}
	}
	
	void KBucket::onResponse(RPCCall* c,MsgBase* rsp)
	{
		last_modified = bt::GetCurrentTime();
		
		if (!pending_entries.contains(c))
			return;
		
		KBucketEntry entry = pending_entries[c];
		pending_entries.erase(c); // call is done so erase it
		
		// we have a response so try to find the next bad or questionable node
		// if we do not have room see if we can get rid of some bad peers
		if (!replaceBadEntry(entry)) // if no bad peers ping a questionable one
			pingQuestionable(entry);
	}
	
	
	
	void KBucket::onTimeout(RPCCall* c)
	{
		if (!pending_entries.contains(c))
			return;
		
		KBucketEntry entry = pending_entries[c];
		
		// replace the entry which timed out
		QValueList<KBucketEntry>::iterator i;
		for (i = entries.begin();i != entries.end();i++)
		{
			KBucketEntry & e = *i;
			if (e.getAddress() == c->getRequest()->getOrigin())
			{
				last_modified = bt::GetCurrentTime();
				entries.remove(i);
				entries.append(entry);
				break;
			}
		}
		pending_entries.erase(c); // call is done so erase it
	}
	
	void KBucket::pingQuestionable(const KBucketEntry & replacement_entry)
	{
		QValueList<KBucketEntry>::iterator i;
		// we haven't found any bad ones so try the questionable ones
		for (i = entries.begin();i != entries.end();i++)
		{
			KBucketEntry & e = *i;
			if (e.isQuestionable())
			{
				PingReq* p = new PingReq(node->getOurID());
				p->setDestination(e.getAddress());
				RPCCall* c = srv->doCall(p);
				if (c)
				{
					c->setListener(this);
					// add the pending entry
					pending_entries.insert(c,replacement_entry);
				}
			}
		}
	}
	
	bool KBucket::replaceBadEntry(const KBucketEntry & entry)
	{
		QValueList<KBucketEntry>::iterator i;
		for (i = entries.begin();i != entries.end();i++)
		{
			KBucketEntry & e = *i;
			if (e.isBad())
			{
				// bad one get rid of it
				last_modified = bt::GetCurrentTime();
				entries.remove(i);
				entries.append(entry);
				return true;
			}
		}
		return false;
	}

	bool KBucket::contains(const KBucketEntry & entry) const
	{
		return entries.contains(entry);
	}
	
	void KBucket::findKClosestNodes(KClosestNodesSearch & kns)
	{
		QValueList<KBucketEntry>::iterator i = entries.begin();
		while (i != entries.end())
		{
			kns.tryInsert(*i);
			i++;
		}
	}
	
	bool KBucket::onTimeout(const KInetSocketAddress & addr)
	{
		QValueList<KBucketEntry>::iterator i;
		
		for (i = entries.begin();i != entries.end();i++)
		{
			KBucketEntry & e = *i;
			if (e.getAddress() == addr)
			{
				e.requestTimeout();
				return true;
			}
		}
		return false;
	}
	
	bool KBucket::needsToBeRefreshed() const
	{
		return (bt::GetCurrentTime() - last_modified > 15 * 60 * 1000);
	}
	
	void KBucket::save(bt::File & fptr)
	{
		QValueList<KBucketEntry>::iterator i;
		
		for (i = entries.begin();i != entries.end();i++)
		{
			KBucketEntry & e = *i;
			const KIpAddress & ip = e.getAddress().ipAddress();
			bt::Uint8 tmp[19];
			if (ip.isIPv4Addr() || ip.isV4Compat())
			{
				tmp[0] = 0x04;
				bt::WriteUint32(tmp,1,ip.IPv4Addr());
				bt::WriteUint16(tmp,5,e.getAddress().port());
				fptr.write(tmp,7);
			}
			else
			{
				tmp[0] = 0x06;
				const sockaddr_in6*  addr = (const sockaddr_in6*)e.getAddress();
				memcpy(tmp+1,addr->sin6_addr.s6_addr,16);
				bt::WriteUint16(tmp,17,e.getAddress().port());
				fptr.write(tmp,19);
			}
		}
	}
	
}

