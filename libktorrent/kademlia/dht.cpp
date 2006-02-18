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
#include <util/array.h>
#include <util/functions.h>
#include <torrent/bnode.h>
#include <torrent/globals.h>
#include <ksocketaddress.h>
#include "dht.h"
#include "node.h"
#include "rpcserver.h"
#include "rpcmsg.h"
#include "kclosestnodessearch.h"
#include "database.h"

using namespace bt;
using namespace KNetwork;

namespace dht
{
	


	DHT::DHT() : node(0),srv(0),db(0)
	{
		node = new Node();
		srv = new RPCServer(this,4444);
		db = new Database();
	}


	DHT::~DHT()
	{
		delete db;
		delete srv;
		delete node;
	}

	void DHT::ping(PingReq* r)
	{
		Out() << "Sending ping response" << endl;
		PingRsp rsp(r->getMTID(),node->getOurID());
		rsp.setOrigin(r->getOrigin());
		srv->sendMsg(&rsp);
		node->recieved(r,srv);
	}
	
	void DHT::ping(PingRsp* r)
	{
		node->recieved(r,srv);
		
	}
	
	void DHT::findNode(FindNodeReq* r)
	{
		node->recieved(r,srv);
		// find the K closest nodes and pack them
		KClosestNodesSearch kns(r->getTarget(),K);
		
		node->findKClosestNodes(kns);
		
		Uint32 rs = kns.requiredSpace();
		// create the data
		QByteArray nodes(rs);
		// pack the found nodes in a byte array
		if (rs > 0)
		{
			Uint8* ptr = (Uint8*)nodes.data();
			kns.pack(ptr,rs);
		}
		
		FindNodeRsp fnr(r->getMTID(),node->getOurID(),nodes);
		fnr.setOrigin(r->getOrigin());
		srv->sendMsg(&fnr);
	}
	
	void DHT::findNode(FindNodeRsp* r)
	{
		node->recieved(r,srv);
	}
	
	void DHT::findValue(FindValueReq* r)
	{
		node->recieved(r,srv);
		const QByteArray & data = db->find(r->getKey());
		if (data.isNull())
		{
			// if data is null do the same as when we have a findNode request
			
			// find the K closest nodes and pack them
			KClosestNodesSearch kns(r->getKey(),K);
		
			node->findKClosestNodes(kns);
		
			Uint32 rs = kns.requiredSpace();
			// create the data
			QByteArray nodes(rs);
			// pack the found nodes in a byte array
			if (rs > 0)
			{
				Uint8* ptr = (Uint8*)nodes.data();
				kns.pack(ptr,rs);
			}
		
			FindNodeRsp fnr(r->getMTID(),node->getOurID(),nodes);
			fnr.setOrigin(r->getOrigin());
			srv->sendMsg(&fnr);
		}
		else
		{
			// send a find value response
			FindValueRsp fvr(r->getMTID(),node->getOurID(),data);
			fvr.setOrigin(r->getOrigin());
			srv->sendMsg(&fvr);
		}
	}
	
	void DHT::findValue(FindValueRsp* r)
	{
		node->recieved(r,srv);
	}
	
	void DHT::storeValue(StoreValueReq* r)
	{
		node->recieved(r,srv);
		// store the key data pair in the db
		db->store(r->getKey(),r->getData());
		
		// send a response
		StoreValueRsp rsp(r->getMTID(),node->getOurID());
		rsp.setOrigin(r->getOrigin());
		srv->sendMsg(&rsp);
	}
	
	void DHT::storeValue(StoreValueRsp* r)
	{
		node->recieved(r,srv);
	}
	
	void DHT::error(ErrMsg* )
	{}

	void DHT::portRecieved(const QString & ip,bt::Uint16 port)
	{
		Out() << "Sending ping request to " << ip << ":" << port << endl;
		PingReq* r = new PingReq(node->getOurID());
		r->setOrigin(KInetSocketAddress(ip,port));
		srv->doCall(r);
	}

}
