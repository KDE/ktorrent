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
#include "kbucket.h"
#include <QtAlgorithms>
#include <QHash>
#include <k3socketaddress.h>
#include <util/file.h>
#include <util/log.h>
#include <util/functions.h>
#include <netinet/in.h>
#include "kclosestnodessearch.h"
#include "rpcserver.h"
#include "node.h"
#include "task.h"

using namespace KNetwork;
using namespace bt;

namespace dht
{
	KBucketEntry::KBucketEntry()
	{
		last_responded = bt::CurrentTime();
		failed_queries = 0;
		questionable_pings = 0;
	}
	
	KBucketEntry::KBucketEntry(const KInetSocketAddress & addr,const Key & id)
	: addr(addr),node_id(id)
	{
		last_responded = bt::CurrentTime();
		failed_queries = 0;
		questionable_pings = 0;
	}
		
	KBucketEntry::KBucketEntry(const KBucketEntry & other)
	: addr(other.addr),node_id(other.node_id),
	last_responded(other.last_responded),failed_queries(other.failed_queries),questionable_pings(other.questionable_pings)
	{}

		
	KBucketEntry::~KBucketEntry()
	{}

	KBucketEntry & KBucketEntry::operator = (const KBucketEntry & other)
	{
		addr = other.addr;
		node_id = other.node_id;
		last_responded = other.last_responded;
		failed_queries = other.failed_queries;
		questionable_pings = other.questionable_pings;
		return *this;
	}
	
	bool KBucketEntry::operator == (const KBucketEntry & entry) const
	{
		return addr == entry.addr && node_id == entry.node_id;
	}
	
	bool KBucketEntry::isGood() const
	{
		if (bt::CurrentTime() - last_responded > 15 * 60 * 1000)
			return false;
		else
			return true;
	}
		
	bool KBucketEntry::isQuestionable() const
	{
		if (bt::CurrentTime() - last_responded > 15 * 60 * 1000)
			return true;
		else
			return false;
	}
		
	
	bool KBucketEntry::isBad() const
	{
		if (isGood())
			return false;
		
		return failed_queries > 2 || questionable_pings > 2;
	}
	
	void KBucketEntry::hasResponded()
	{
		last_responded = bt::CurrentTime();
		failed_queries = 0; // reset failed queries
		questionable_pings = 0;
	}
	
	bool KBucketEntry::operator<(const dht::KBucketEntry& entry) const
	{
		return node_id < entry.node_id;
	}


	//////////////////////////////////////////////////////////
	
	KBucket::KBucket(Uint32 idx,RPCServer* srv,Node* node) 
		: idx(idx),srv(srv),node(node)
	{
		last_modified = bt::CurrentTime();
		refresh_task = 0;
	}
	
	
	KBucket::~KBucket()
	{}
	
	void KBucket::insert(const KBucketEntry & entry)
	{
		QList<KBucketEntry>::iterator i = qFind(entries.begin(),entries.end(),entry);
	
		// If in the list, move it to the end
		if (i != entries.end())
		{
			KBucketEntry & e = *i;
			e.hasResponded();
			last_modified = bt::CurrentTime();
			entries.erase(i);
			entries.append(entry);
			return;
		}
		
		// insert if not already in the list and we still have room
		if (i == entries.end() && entries.count() < (int) dht::K)
		{
			entries.append(entry);
			last_modified = bt::CurrentTime();
		}
		else if (!replaceBadEntry(entry))
		{
			// ping questionable nodes when replacing a bad one fails
			pingQuestionable(entry);	
		}
	}
	
	void KBucket::onResponse(RPCCall* c,MsgBase* rsp)
	{
		Q_UNUSED(rsp);
		last_modified = bt::CurrentTime();
		
		if (!pending_entries_busy_pinging.contains(c))
			return;
		
		KBucketEntry entry = pending_entries_busy_pinging[c];
		pending_entries_busy_pinging.remove(c); // call is done so erase it
		
		// we have a response so try to find the next bad or questionable node
		// if we do not have room see if we can get rid of some bad peers
		if (!replaceBadEntry(entry)) // if no bad peers ping a questionable one
			pingQuestionable(entry);
		
	}
	
	
	
	void KBucket::onTimeout(RPCCall* c)
	{
		if (!pending_entries_busy_pinging.contains(c))
			return;
		
		KBucketEntry entry = pending_entries_busy_pinging[c];
		
		// replace the entry which timed out
		QList<KBucketEntry>::iterator i;
		for (i = entries.begin();i != entries.end();i++)
		{
			KBucketEntry & e = *i;
			if (e.getAddress() == c->getRequest()->getOrigin())
			{
				last_modified = bt::CurrentTime();
				entries.erase(i);
				entries.append(entry);
				break;
			}
		}
		pending_entries_busy_pinging.remove(c); // call is done so erase it
		// see if we can do another pending entry
		if (pending_entries_busy_pinging.count() < 2 && pending_entries.count() > 0)
		{
			KBucketEntry pe = pending_entries.front();
			pending_entries.pop_front();
			if (!replaceBadEntry(pe)) // if no bad peers ping a questionable one
				pingQuestionable(pe);
		}
	}
	
	void KBucket::pingQuestionable(const KBucketEntry & replacement_entry)
	{
		if (pending_entries_busy_pinging.count() >= 2)
		{
			pending_entries.append(replacement_entry); // lets not have to many pending_entries calls going on
			return;
		}
		
		QList<KBucketEntry>::iterator i;
		// we haven't found any bad ones so try the questionable ones
		for (i = entries.begin();i != entries.end();i++)
		{
			KBucketEntry & e = *i;
			if (e.isQuestionable())
			{
				Out(SYS_DHT|LOG_DEBUG) << "Pinging questionable node : " << e.getAddress().toString() << endl;
				PingReq* p = new PingReq(node->getOurID());
				p->setDestination(e.getAddress());
				RPCCall* c = srv->doCall(p);
				if (c)
				{
					e.onPingQuestionable();
					c->addListener(this);
					// add the pending entry
					pending_entries_busy_pinging.insert(c,replacement_entry);
					return;
				}
			}
		}
	}
	
	bool KBucket::replaceBadEntry(const KBucketEntry & entry)
	{
		QList<KBucketEntry>::iterator i;
		for (i = entries.begin();i != entries.end();i++)
		{
			KBucketEntry & e = *i;
			if (e.isBad())
			{
				// bad one get rid of it
				last_modified = bt::CurrentTime();
				entries.erase(i);
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
		QList<KBucketEntry>::iterator i = entries.begin();
		while (i != entries.end())
		{
			kns.tryInsert(*i);
			i++;
		}
	}
	
	bool KBucket::onTimeout(const KInetSocketAddress & addr)
	{
		QList<KBucketEntry>::iterator i;
		
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
		bt::TimeStamp now = bt::CurrentTime();
		if (last_modified > now)
		{
			last_modified = now;
			return false;
		}
		
		return !refresh_task && entries.count() > 0 && (now - last_modified > BUCKET_REFRESH_INTERVAL);
	}
	
	void KBucket::updateRefreshTimer()
	{
		last_modified = bt::CurrentTime();
	}
	
	
	
	void KBucket::save(bt::File & fptr)
	{
		BucketHeader hdr;
		hdr.magic = BUCKET_MAGIC_NUMBER + 1;
		hdr.index = idx;
		hdr.num_entries = entries.count();
		
		fptr.write(&hdr,sizeof(BucketHeader));
		QList<KBucketEntry>::iterator i;
		for (i = entries.begin();i != entries.end();i++)
		{
			KBucketEntry & e = *i;
			const KIpAddress & ip = e.getAddress().ipAddress();
			
			if (ip.version() == 4)
			{
				Uint8 tmp[27];
				tmp[0] = 0x04;
				bt::WriteUint32(tmp,1,ip.IPv4Addr());
				bt::WriteUint16(tmp,5,e.getAddress().port());
				memcpy(tmp+7,e.getID().getData(),20);
				fptr.write(tmp,27);
			}
			else
			{
				Uint8 tmp[39];
				tmp[0] = 0x06;
				memcpy(tmp+1,ip.addr(),16);
				bt::WriteUint16(tmp,17,e.getAddress().port());
				memcpy(tmp+19,e.getID().getData(),20);
				fptr.write(tmp,39);
			}
		}
	}
	
	void KBucket::load(bt::File & fptr,const BucketHeader & hdr)
	{
		if (hdr.num_entries > K)
			return;
		
		for (Uint32 i = 0;i < hdr.num_entries;i++)
		{
			Uint8 tmp[39];
			if (fptr.read(tmp,27) != 27)
				return;
			
			if (tmp[0] == 0x06) // IPv6, so read more
				if (fptr.read(tmp+27,12) != 12)
					return;
			
			if (tmp[0] == 0x06)
			{
				entries.append(KBucketEntry(
					KInetSocketAddress(
						KIpAddress(tmp+1,6),
						bt::ReadUint16(tmp,17)),
					dht::Key(tmp+19)));
			}
			else
			{
				entries.append(KBucketEntry(
					KInetSocketAddress(
						KIpAddress(bt::ReadUint32(tmp,1)),
						bt::ReadUint16(tmp,5)),
					dht::Key(tmp+7)));
			}
		}
	}
	
	void KBucket::onFinished(Task* t)
	{
		if (t == refresh_task)
			refresh_task = 0;
	}
	
	void KBucket::setRefreshTask(Task* t)
	{
		refresh_task = t;
		if (refresh_task)
		{
			connect(refresh_task,SIGNAL(finished( Task* )),
					this,SLOT(onFinished( Task* )));
		}
	}
	
}

#include "kbucket.moc"
