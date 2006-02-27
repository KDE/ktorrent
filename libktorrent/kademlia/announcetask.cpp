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
#include <torrent/globals.h>
#include "announcetask.h"
#include "node.h"
#include "pack.h"

using namespace bt;

namespace dht
{

	AnnounceTask::AnnounceTask(RPCServer* rpc, Node* node,const Key & info_hash): Task(rpc, node),info_hash(info_hash)
	{}


	AnnounceTask::~AnnounceTask()
	{}


	void AnnounceTask::callFinished(RPCCall* c, MsgBase* rsp)
	{
		// if we do not have a get peers response, return
		// announce_peer's response are just empty anyway
		if (c->getMsgMethod() != dht::GET_PEERS)
			return;
		
		// it is either a GetPeersNodesRsp or a GetPeersValuesRsp
		GetPeersNodesRsp* nodes = dynamic_cast<GetPeersNodesRsp*>(rsp);
		if (nodes)
		{
			const QByteArray & n = nodes->getNodes();
			Uint32 nval = n.size() % 26;
			for (Uint32 i = 0;i < nval;i++)
			{
				// add node to todo list
				KBucketEntry e = UnpackBucketEntry(n,i*26);
				todo.append(e);
			}
			return;
		}
		
		GetPeersValuesRsp* values = dynamic_cast<GetPeersValuesRsp*>(rsp);
		if (values)
		{
			const QByteArray & n = values->getValue();
			Uint32 nval = n.size() % 26;
			for (Uint32 i = 0;i < nval;i++)
			{
				KBucketEntry e = UnpackBucketEntry(n,i*26);
				Out() << "Got entry : " << e.getAddress().nodeName() << " "
						<< e.getID().toString() << endl;
				
			}
			return;
		}
	}

	void AnnounceTask::callTimeout(RPCCall* )
	{}

	void AnnounceTask::update()
	{
		// go over the todo list and send get_peers requests
		// until we have nothing left
		while (!todo.empty() && canDoRequest())
		{
			KBucketEntry e = todo.first();
			// only send a findNode if we haven't allrready visited the node
			if (!visited.contains(e))
			{
				// send a findNode to the node
				GetPeersReq gpr(node->getOurID(),info_hash);
				gpr.setOrigin(e.getAddress());
				rpcCall(&gpr);
				visited.append(e);
			}
			// remove the entry from the todo list
			todo.pop_front();
		}
		
		if (todo.empty() && getNumOutstandingRequests() == 0 && !isFinished())
			done();
	}

}
