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
#include <util/log.h>
#include <torrent/globals.h>
#include "nodelookup.h"
#include "rpcmsg.h"
#include "node.h"
#include "pack.h"

using namespace bt;

namespace dht
{

	NodeLookup::NodeLookup(const dht::Key & key,RPCServer* rpc,Node* node) 
	: Task(rpc,node),node_id(key),num_nodes_rsp(0)
	{
	}


	NodeLookup::~NodeLookup()
	{}


	void NodeLookup::callFinished(RPCCall* ,MsgBase* rsp)
	{
	//	Out() << "NodeLookup::callFinished" << endl;
		if (isFinished())
			return;
		
		// check the response and see if it is a good one
		if (rsp->getMethod() == dht::FIND_NODE && rsp->getType() == dht::RSP_MSG)
		{
			FindNodeRsp* fnr = (FindNodeRsp*)rsp;
			const QByteArray & nodes = fnr->getNodes();
			Uint32 nnodes = nodes.size() / 26;
			for (Uint32 j = 0;j < nnodes;j++)
			{
				// unpack an entry and add it to the todo list
				KBucketEntry e = UnpackBucketEntry(nodes,j*26);
				// lets not talk to ourself
				if (e.getID() != node->getOurID() && !todo.contains(e) && !visited.contains(e))
					todo.append(e);
			}
			num_nodes_rsp++;
		}
	}
	
	void NodeLookup::callTimeout(RPCCall*)
	{
	//	Out() << "NodeLookup::callTimeout" << endl;
	}
	
	void NodeLookup::update()
	{
	//	Out() << "NodeLookup::update" << endl;
	//	Out() << "todo = " << todo.count() << " ; visited = " << visited.count() << endl;
		// go over the todo list and send find node calls
		// until we have nothing left
		while (!todo.empty() && canDoRequest())
		{
			KBucketEntry e = todo.first();
			// only send a findNode if we haven't allrready visited the node
			if (!visited.contains(e))
			{
				// send a findNode to the node
				FindNodeReq* fnr = new FindNodeReq(node->getOurID(),node_id);
				fnr->setOrigin(e.getAddress());
				rpcCall(fnr);
				visited.append(e);
			}
			// remove the entry from the todo list
			todo.pop_front();
		}
		
		if (todo.empty() && getNumOutstandingRequests() == 0 && !isFinished())
			done();
		else if (num_nodes_rsp > 50)
			done(); // quit after 50 nodes responses
	}
}
