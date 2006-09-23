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
#include "announcetask.h"
#include "node.h"
#include "pack.h"

using namespace bt;

namespace dht
{

	AnnounceTask::AnnounceTask(Database* db,RPCServer* rpc, Node* node,const dht::Key & info_hash,bt::Uint16 port)
	: Task(rpc, node),info_hash(info_hash),port(port),db(db)
	{}


	AnnounceTask::~AnnounceTask()
	{}


	void AnnounceTask::callFinished(RPCCall* c, MsgBase* rsp)
	{
	//	Out() << "AnnounceTask::callFinished" << endl;
		// if we do not have a get peers response, return
		// announce_peer's response are just empty anyway
		if (c->getMsgMethod() != dht::GET_PEERS)
			return;
		
		// it is either a GetPeersNodesRsp or a GetPeersValuesRsp
		GetPeersRsp* gpr = dynamic_cast<GetPeersRsp*>(rsp);
		if (!gpr)
			return;
		
		if (gpr->containsNodes())
		{
			const QByteArray & n = gpr->getData();
			Uint32 nval = n.size() / 26;
			for (Uint32 i = 0;i < nval;i++)
			{
				// add node to todo list
				KBucketEntry e = UnpackBucketEntry(n,i*26);
				if (!todo.contains(e) && !visited.contains(e) && 
					todo.count() < 100)
				{
					todo.append(e);
				}
			}
		}
		else
		{
			// store the items in the database
			const DBItemList & items = gpr->getItemList();
			for (DBItemList::const_iterator i = items.begin();i != items.end();i++)
			{
				db->store(info_hash,*i);
				// also add the items to the returned_items list
				returned_items.append(*i);
			}
			
			// add the peer who responded to the answered list, so we can do an announce
			KBucketEntry e(rsp->getOrigin(),rsp->getID());
			if (!answered.contains(KBucketEntryAndToken(e,gpr->getToken())) && !answered_visited.contains(e))
			{
				answered.append(KBucketEntryAndToken(e,gpr->getToken()));
			}
			
			emitDataReady();
		}
	}

	void AnnounceTask::callTimeout(RPCCall* )
	{
		//Out() << "AnnounceTask::callTimeout " << endl;
	}

	void AnnounceTask::update()
	{
/*		Out() << "AnnounceTask::update " << endl;
		Out() << "todo " << todo.count() << " ; answered " << answered.count() << endl;
		Out() << "visited " << visited.count() << " ; answered_visited " << answered_visited.count() << endl;
	*/
		while (!answered.empty() && canDoRequest())
		{
			KBucketEntryAndToken & e = answered.first();
			if (!answered_visited.contains(e))
			{
				AnnounceReq* anr = new AnnounceReq(node->getOurID(),info_hash,port,e.getToken());
				anr->setOrigin(e.getAddress());
				rpcCall(anr);
				answered_visited.append(e);
			}
			answered.pop_front();
		}
		
		// go over the todo list and send get_peers requests
		// until we have nothing left
		while (!todo.empty() && canDoRequest())
		{
			KBucketEntry e = todo.first();
			// onLy send a findNode if we haven't allrready visited the node
			if (!visited.contains(e))
			{
				// send a findNode to the node
				GetPeersReq* gpr = new GetPeersReq(node->getOurID(),info_hash);
				gpr->setOrigin(e.getAddress());
				rpcCall(gpr);
				visited.append(e);
			}
			// remove the entry from the todo list
			todo.pop_front();
		}
		
		if (todo.empty() && answered.empty() && getNumOutstandingRequests() == 0 && !isFinished())
		{
			Out(SYS_DHT|LOG_NOTICE) << "DHT: AnnounceTask done" << endl;
			done();
		}
		else if (answered_visited.count() >= dht::K)
		{
			// if K announces have occurred stop
			Out(SYS_DHT|LOG_NOTICE) << "DHT: AnnounceTask done" << endl;
			done();
		}
	}

	bool AnnounceTask::takeItem(DBItem & item)
	{
		if (returned_items.empty())
			return false;
		
		item = returned_items.first();
		returned_items.pop_front();
		return true;	
	}
}
