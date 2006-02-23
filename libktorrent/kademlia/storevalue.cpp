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
#include "storevalue.h"
#include "node.h"

namespace dht
{
	

	StoreValue::StoreValue(const dht::Key & key,const QByteArray & data,RPCServer* rpc,Node* node)
	: Task(rpc,node),key(key),data(data),succesfull_stores(0)
	{}


	StoreValue::~StoreValue()
	{}


	void StoreValue::callFinished(RPCCall* , MsgBase* rsp)
	{
		if (rsp->getType() == dht::RSP_MSG && rsp->getMethod() == dht::STORE_VALUE)
		{
			succesfull_stores++;
		}
	}

	void StoreValue::callTimeout(RPCCall*)
	{}

	void StoreValue::update()
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
				StoreValueReq str(node->getOurID(),key,data);
				str.setOrigin(e.getAddress());
				rpcCall(&str);
				visited.append(e);
			}
			// remove the entry from the todo list
			todo.pop_front();
		}
		
		if (succesfull_stores >= STORE_REDUNDANCY && !isFinished())
			done();
	}

}
