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
#include "valuelookup.h"
#include "node.h"
#include "pack.h"

namespace dht
{

	ValueLookup::ValueLookup(const dht::Key & key,RPCServer* rpc,Node* node): Task(rpc,node),key(key)
	{}


	ValueLookup::~ValueLookup()
	{}


	void ValueLookup::callFinished(RPCCall* , MsgBase* rsp)
	{
		if (isFinished() || rsp->getType() != dht::RSP_MSG)
			return;
			
		if (rsp->getMethod() == dht::FIND_NODE)
		{
			// the node doesn't know the key, so it gives back a series of node to further query
			FindNodeRsp* fnr = (FindNodeRsp*)rsp;
			const QByteArray & nodes = fnr->getNodes();
			Uint32 nnodes = nodes.size() / 26;
			for (Uint32 j = 0;j < nnodes;j++)
			{
				// unpack an entry and add it to the todo list
				KBucketEntry e = UnpackBucketEntry(nodes,j*26);
				// lets not talk to ourself
				if (e.getID() != node->getOurID())
					todo.append(e);
			}
		}
		else if (rsp->getMethod() == dht::FIND_VALUE)
		{
			// The node has the value
			FindValueRsp* fv = (FindValueRsp*)rsp;
			value = fv->getValue();
			// clear todo list, the task is finished
			todo.clear();
		}
	}

	void ValueLookup::callTimeout(RPCCall*)
	{
	}

	void ValueLookup::update()
	{
		// go over the todo list and send find value calls
		// until we have nothing left
		while (!todo.empty() && canDoRequest())
		{
			KBucketEntry e = todo.first();
			// only send a findNode if we haven't allrready visited the node
			if (!visited.contains(e))
			{
				// send a findValue to the node
				FindValueReq fnr(node->getOurID(),key);
				fnr.setOrigin(e.getAddress());
				rpcCall(&fnr);
				visited.append(e);
			}
			// remove the entry from the todo list
			todo.pop_front();
		}
	}

}
