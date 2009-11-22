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
#include "announcetask.h"
#include <util/log.h>
#include <torrent/globals.h>
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
		//Out(SYS_DHT|LOG_DEBUG) << "AnnounceTask::callFinished" << endl;
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
			const QByteArray & n = gpr->getNodes();
			Uint32 nval = n.size() / 26;
			for (Uint32 i = 0;i < nval;i++)
			{
				// add node to todo list
				try
				{
					KBucketEntry e = UnpackBucketEntry(n,i*26,4);
					if (!visited.contains(e) && todo.size() < 100)
					{
						todo.insert(e);
					//	Out(SYS_DHT|LOG_DEBUG) << "DHT: GetPeers returned node " << e.getAddress().toString() << endl;
					}
				}
				catch (...)
				{
					// not enough size in buffer, just ignore the error
				}
			}
			
			for (PackedNodeContainer::CItr itr = gpr->begin();itr != gpr->end();itr++)
			{
				const QByteArray & ba = *itr;
				try
				{
					KBucketEntry e = UnpackBucketEntry(ba,0,6);
					if (!visited.contains(e) && todo.size() < 100)
					{
						todo.insert(e);
					//	Out(SYS_DHT|LOG_DEBUG) << "DHT: GetPeers returned node " << e.getAddress().toString() << endl;
					}
				}
				catch (...)
				{
					// bad data, ignore
				}
			}
		}
		else
		{
			// store the items in the database
			const DBItemList & items = gpr->getItemList();
			for (DBItemList::const_iterator i = items.begin();i != items.end();i++)
			{
			//	Out(SYS_DHT|LOG_DEBUG) << "DHT: GetPeers returned item " << i->getAddress().toString() << endl;
				db->store(info_hash,*i);
				// also add the items to the returned_items list
				returned_items.append(*i);
			}
			
			emitDataReady();
		}
		
		// add the peer who responded to the answered list, so we can do an announce
		KBucketEntry e(rsp->getOrigin(),rsp->getID());
		if (!answered_visited.contains(e))
		{
			answered.insert(KBucketEntryAndToken(e,gpr->getToken()));
		}
	}

	void AnnounceTask::callTimeout(RPCCall* )
	{
		//Out(SYS_DHT|LOG_DEBUG) << "AnnounceTask::callTimeout " << endl;
	}

	void AnnounceTask::update()
	{
	/*	Out(SYS_DHT|LOG_DEBUG) << "AnnounceTask::update " << endl;
		Out(SYS_DHT|LOG_DEBUG) << "todo " << todo.count() << " ; answered " << answered.count() << endl;
		Out(SYS_DHT|LOG_DEBUG) << "visited " << visited.count() << " ; answered_visited " << answered_visited.count() << endl;
	*/
		while (!answered.empty() && canDoRequest())
		{
			std::set<KBucketEntryAndToken>::iterator itr = answered.begin();
			if (!answered_visited.contains(*itr))
			{
				AnnounceReq* anr = new AnnounceReq(node->getOurID(),info_hash,port,itr->getToken());
				anr->setOrigin(itr->getAddress());
		//		Out(SYS_DHT|LOG_DEBUG) << "DHT: Announcing to " << e.getAddress().toString() << endl;
				rpcCall(anr);
				answered_visited.insert(*itr);
			}
			answered.erase(itr);
		}
		
		// go over the todo list and send get_peers requests
		// until we have nothing left
		while (!todo.empty() && canDoRequest())
		{
			KBucketEntrySet::iterator itr = todo.begin();
			// onLy send a findNode if we haven't allrready visited the node
			if (!visited.contains(*itr))
			{
				// send a findNode to the node
		//		Out(SYS_DHT|LOG_DEBUG) << "DHT: Sending GetPeers to " << e.getAddress().toString() << endl;
				GetPeersReq* gpr = new GetPeersReq(node->getOurID(),info_hash);
				gpr->setOrigin(itr->getAddress());
				rpcCall(gpr);
				visited.insert(*itr);
			}
			// remove the entry from the todo list
			todo.erase(itr);
		}
		
		
		if (todo.empty() && answered.empty() && getNumOutstandingRequests() == 0 && !isFinished())
		{
			Out(SYS_DHT|LOG_NOTICE) << "DHT: AnnounceTask done" << endl;
			done();
		}
		else if (answered_visited.size() > 50 || visited.size() > 200)
		{
			// don't let the task run forever
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
