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
#include "node.h"
#include "rpcmsg.h"
#include "key.h"
#include "rpccall.h"
#include "rpcserver.h"
#include "kclosestnodessearch.h"


namespace dht
{

	Node::Node()
	{
		our_id = dht::Key::random();
		for (int i = 0;i < 160;i++)
			bucket[i] = 0;
	}


	Node::~Node()
	{
		for (int i = 0;i < 160;i++)
			delete bucket[i];
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

	void Node::recieved(const MsgBase* msg,RPCServer* srv)
	{
		Uint8 bit_on = findBucket(msg->getID());
		
		// return if bit_on is not good
		if (bit_on >= 160)
			return;
		
		// make the bucket if it doesn't exist
		if (!bucket[bit_on])
			bucket[bit_on] = new KBucket();
		
		// try to insert it into the bucket
		KBucket* kb = bucket[bit_on];
		
		// try to insert it into the bucket
		if (!kb->insert(KBucketEntry(msg->getOrigin(),msg->getID())))
		{
			// insert failed, bucket is full
			PingReq* p = new PingReq(our_id);
			RPCCall* c = srv->doCall(p);
			if (c)
				c->setListener(this);
		}
	}
	
	void Node::onResponse(RPCCall*,MsgBase* rsp)
	{
		Uint8 bit_on = findBucket(rsp->getID());
		
		// return if bit_on is not good
		if (bit_on >= 160)
			return;
		
		// make the bucket if it doesn't exist
		if (!bucket[bit_on])
			bucket[bit_on] = new KBucket();
		
		
		KBucket* kb = bucket[bit_on];
		// insert it into the bucket
		kb->insert(KBucketEntry(rsp->getOrigin(),rsp->getID()),true);
	}
	
	void Node::onTimeout(RPCCall* ) {}

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
}

#include "node.moc"
