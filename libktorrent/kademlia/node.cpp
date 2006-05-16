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

#include <util/log.h>
#include <util/file.h>
#include <util/functions.h>
#include <torrent/globals.h>
#include "node.h"
#include "rpcmsg.h"
#include "key.h"
#include "rpccall.h"
#include "rpcserver.h"
#include "kclosestnodessearch.h"
#include "dht.h"
#include "nodelookup.h"

using namespace bt;
using namespace KNetwork;

namespace dht
{

	Node::Node(RPCServer* srv) : srv(srv)
	{
		num_receives = 0;
		num_entries = 0;
		our_id = dht::Key::random();
		for (int i = 0;i < 160;i++)
			bucket[i] = 0;
	}


	Node::~Node()
	{
		for (int i = 0;i < 160;i++)
		{
			KBucket* b = bucket[i];
			if (b)
				delete b;
		}
	}
	
	Uint8 Node::findBucket(const dht::Key & id)
	{
		// XOR our id and the sender's ID
		dht::Key d = dht::Key::distance(id,our_id);
		// now use the first on bit to determin which bucket it should go in
		
		Uint8 bit_on = 0xFF;
		for (Uint32 i = 0;i < 20;i++)
		{
			// get the byte
			Uint8 b = *(d.getData() + i);
			// no bit on in this byte so continue
			if (b == 0x00)
				continue;
			
			for (Uint8 j = 0;j < 8;j++)
			{
				if (b & (0x80 >> j))
				{
					// we have found the bit
					bit_on = (19 - i)*8 + (7 - j);
				}
			}
		}
		return bit_on;
	}

	void Node::recieved(DHT* dh_table,const MsgBase* msg)
	{
		Uint8 bit_on = findBucket(msg->getID());
		
		// return if bit_on is not good
		if (bit_on >= 160)
			return;
		
		// make the bucket if it doesn't exist
		if (!bucket[bit_on])
			bucket[bit_on] = new KBucket(bit_on,srv,this);
		
		// insert it into the bucket
		KBucket* kb = bucket[bit_on];
		kb->insert(KBucketEntry(msg->getOrigin(),msg->getID()));
		num_receives++;
		if (num_receives == 3)
		{
			// do a node lookup upon our own id 
			// when we insert the first entry in the table
			dh_table->findNode(our_id);
		}
		
		num_entries = 0;
		for (Uint32 i = 0;i < 160;i++)
			if (bucket[i])
				num_entries += bucket[i]->getNumEntries();
	}

	void Node::findKClosestNodes(KClosestNodesSearch & kns)
	{
		// go over all buckets until
		for (Uint32 i = 0;i < 160;i++)
		{
			if (bucket[i])
			{
				bucket[i]->findKClosestNodes(kns);
			}
		}
	}
	
	void Node::onTimeout(const MsgBase* msg)
	{
		for (Uint32 i = 0;i < 160;i++)
		{
			if (bucket[i] && bucket[i]->onTimeout(msg->getDestination()))
			{
				return;
			}
		}
	}
	
	/// Generate a random key which lies in a certain bucket
	Key RandomKeyInBucket(Uint32 b,const Key & our_id)
	{
		// first generate a random one
		Key r = dht::Key::random();
		Uint8* data = (Uint8*)r.getData();
		
		// before we hit bit b, everything needs to be equal to our_id
		Uint8 nb = b / 8;
		for (Uint8 i = 0;i < nb;i++)
			data[i] = *(our_id.getData() + i);
		
		
		// copy all bits of ob, until we hit the bit which needs to be different
		Uint8 ob = *(our_id.getData() + nb);
		for (Uint8 j = 0;j < b % 8;j++)
		{
			if ((0x80 >> j) & ob)
				data[nb] |= (0x80 >> j);
			else
				data[nb] &= ~(0x80 >> j);
		}
		
		// if the bit b is on turn it off else turn it on
		if ((0x80 >> (b % 8)) & ob)
			data[nb] &= ~(0x80 >> (b % 8));
		else
			data[nb] |= (0x80 >> (b % 8));
		
		return Key(data);
	}
	
	void Node::refreshBuckets(DHT* dh_table)
	{
		for (Uint32 i = 0;i < 160;i++)
		{
			KBucket* b = bucket[i];
			if (b && b->needsToBeRefreshed())
			{
				// the key needs to be the refreshed
				NodeLookup* nl = dh_table->refreshBucket(RandomKeyInBucket(i,our_id),*b);
				if (nl)
					b->setRefreshTask(nl);
			}
		}
	}
	
	
	void Node::saveTable(const QString & file)
	{
		bt::File fptr;
		if (!fptr.open(file,"wb"))
		{
			Out() << "DHT: Cannot open file " << file << " : " << fptr.errorString() << endl;
			return;
		}
		
		for (Uint32 i = 0;i < 160;i++)
		{
			KBucket* b = bucket[i];
			if (b)
			{
				b->save(fptr);
			}
		}
	}
		
	void Node::loadTable(const QString & file)
	{
		bt::File fptr;
		if (!fptr.open(file,"rb"))
		{
			Out() << "DHT: Cannot open file " << file << " : " << fptr.errorString() << endl;
			return;
		}
		
		num_entries = 0;
		while (!fptr.eof())
		{
			BucketHeader hdr;
			if (fptr.read(&hdr,sizeof(BucketHeader)) != sizeof(BucketHeader))
				return;
			
			if (hdr.magic != dht::BUCKET_MAGIC_NUMBER || hdr.num_entries > dht::K || hdr.index > 160)
				return;
			
			if (hdr.num_entries == 0)
				continue;
			
			Out() << "DHT: Loading bucket " << hdr.index << endl;
			if (bucket[hdr.index])
				delete bucket[hdr.index];
			
			bucket[hdr.index] = new KBucket(hdr.index,srv,this);
			bucket[hdr.index]->load(fptr,hdr);
			num_entries += bucket[hdr.index]->getNumEntries();
		}
	}
}

#include "node.moc"
