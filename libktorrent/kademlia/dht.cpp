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
#include "announcetask.h"
#include "dht.h"
#include "node.h"
#include "rpcserver.h"
#include "rpcmsg.h"
#include "kclosestnodessearch.h"
#include "database.h"
#include "taskmanager.h"


using namespace bt;
using namespace KNetwork;

namespace dht
{
	


	DHT::DHT() : node(0),srv(0),db(0),tman(0),running(false),port(6881)
	{
		
	}


	DHT::~DHT()
	{
		delete tman;
		delete db;
		delete srv;
		delete node;
	}
	
	void DHT::start(bt::Uint16 port)
	{
		if (running)
			return;
		
		this->port = port;
		Out() << "Starting DHT on port " << port << endl;
		node = new Node();
		srv = new RPCServer(this,port);
		db = new Database();
		tman = new TaskManager();
		expire_timer.update();
		running = true;
	}
		
		
	void DHT::stop()
	{
		if (!running)
			return;
		
		Out() << "Stopping dht" << endl;
		delete tman; tman = 0;
		delete db; db = 0;
		delete srv; srv = 0;
		delete node; node = 0;
		running = false;
	}

	void DHT::ping(PingReq* r)
	{
		if (!running)
			return;
		
		Out() << "Sending ping response" << endl;
		PingRsp rsp(r->getMTID(),node->getOurID());
		rsp.setOrigin(r->getOrigin());
		srv->sendMsg(&rsp);
		node->recieved(r,srv);
	}
	
	void DHT::findNode(FindNodeReq* r)
	{
		if (!running)
			return;
		
		node->recieved(r,srv);
		// find the K closest nodes and pack them
		KClosestNodesSearch kns(r->getTarget(),K);
		
		node->findKClosestNodes(kns);
		
		Uint32 rs = kns.requiredSpace();
		// create the data
		QByteArray nodes(rs);
		// pack the found nodes in a byte array
		if (rs > 0)
			kns.pack(nodes);
		
		FindNodeRsp fnr(r->getMTID(),node->getOurID(),nodes);
		fnr.setOrigin(r->getOrigin());
		srv->sendMsg(&fnr);
	}
	
	void DHT::findValue(FindValueReq* r)
	{
		if (!running)
			return;
		
		node->recieved(r,srv);
#if 0
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
				kns.pack(nodes);
		
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
#endif
	}
	
	void DHT::storeValue(StoreValueReq* r)
	{
		if (!running)
			return;
		
		node->recieved(r,srv);
		// store the key data pair in the db
		//db->store(r->getKey(),r->getData());
		
		// send a response
		StoreValueRsp rsp(r->getMTID(),node->getOurID());
		rsp.setOrigin(r->getOrigin());
		srv->sendMsg(&rsp);
	}
	
	void DHT::announce(AnnounceReq* r)
	{
		if (!running)
			return;
		
		node->recieved(r,srv);
		// first check if the token is OK
		dht::Key token = r->getToken();
		if (!db->checkToken(token,r->getOrigin().ipAddress().IPv4Addr(),r->getOrigin().port()))
			return;
		
		// everything OK, so store the value
		Uint8 tdata[6];
		bt::WriteUint32(tdata,0,r->getOrigin().ipAddress().IPv4Addr());
		bt::WriteUint16(tdata,4,r->getPort());
		db->store(r->getInfoHash(),DBItem(tdata));
		// send a proper response to indicate everything is OK
		AnnounceRsp rsp(r->getMTID(),node->getOurID());
		srv->sendMsg(&rsp);
	}
	
	
	
	void DHT::getPeers(GetPeersReq* r)
	{
		if (!running)
			return;
		
		node->recieved(r,srv);
		DBItemList dbl;
		db->sample(r->getInfoHash(),dbl,50);
		
		// generate a token
		Key token = db->genToken(r->getOrigin().ipAddress().IPv4Addr(),r->getOrigin().port());
		
		if (dbl.count() == 0)
		{
			// if data is null do the same as when we have a findNode request
			// find the K closest nodes and pack them
			KClosestNodesSearch kns(r->getInfoHash(),K);
			node->findKClosestNodes(kns);
			Uint32 rs = kns.requiredSpace();
			// create the data
			QByteArray nodes(rs);
			// pack the found nodes in a byte array
			if (rs > 0)
				kns.pack(nodes);
		
			GetPeersRsp fnr(r->getMTID(),node->getOurID(),nodes,token);
			fnr.setOrigin(r->getOrigin());
			srv->sendMsg(&fnr);
		}
		else
		{			
			// send a get peers response
			GetPeersRsp fvr(r->getMTID(),node->getOurID(),dbl,token);
			fvr.setOrigin(r->getOrigin());
			srv->sendMsg(&fvr);
		}
	}
	
	void DHT::response(MsgBase* r)
	{
		if (!running)
			return;
		
		node->recieved(r,srv);
	}
	
	void DHT::error(ErrMsg* )
	{}
	

	void DHT::portRecieved(const QString & ip,bt::Uint16 port)
	{
		if (!running)
			return;
		
		Out() << "Sending ping request to " << ip << ":" << port << endl;
		PingReq* r = new PingReq(node->getOurID());
		r->setOrigin(KInetSocketAddress(ip,port));
		srv->doCall(r);
	}
	
	AnnounceTask* DHT::announce(const bt::SHA1Hash & info_hash,bt::Uint16 port)
	{
		if (!running)
			return 0;
		
		KClosestNodesSearch kns(info_hash,K);
		node->findKClosestNodes(kns);
		if (kns.getNumEntries() > 0)
		{
			Out() << "Doing DHT announce " << endl;
			AnnounceTask* at = new AnnounceTask(db,srv,node,info_hash,port);
			tman->addTask(at);
			at->start(kns);
			return at;
		}
		
		return 0;
	}
	
	void DHT::update()
	{
		if (!running)
			return;
		
		if (expire_timer.getElapsedSinceUpdate() > 5*60*1000)
		{
			db->expire(bt::GetCurrentTime());
			expire_timer.update();
		}
	}
}
