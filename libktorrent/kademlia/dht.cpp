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
#include <torrent/bnode.h>
#include <torrent/globals.h>
#include <ksocketaddress.h>
#include "dht.h"
#include "node.h"
#include "rpcserver.h"
#include "rpcmsg.h"

using namespace bt;
using namespace KNetwork;

namespace dht
{


	DHT::DHT() : node(0),srv(0),mtid(0)
	{
		node = new Node();
		srv = new RPCServer(this,4444);
	}


	DHT::~DHT()
	{
		delete srv;
		delete node;
	}

	void DHT::ping(PingReq* r)
	{
		Out() << "Sending ping response" << endl;
		PingRsp* rsp = new PingRsp(r->getMTID(),node->getOurID());
		rsp->setOrigin(r->getOrigin());
		srv->sendMsg(rsp);
		delete rsp;
		node->recieved(r,srv,mtid);
	}
	
	void DHT::ping(PingRsp* r)
	{
		node->recieved(r,srv,mtid);
	}
	
	void DHT::findNode(FindNodeReq* r)
	{
		node->recieved(r,srv,mtid);
	}
	
	void DHT::findNode(FindNodeRsp* r)
	{
		node->recieved(r,srv,mtid);
	}
	
	void DHT::findValue(FindValueReq* r)
	{
		node->recieved(r,srv,mtid);
	}
	
	void DHT::findValue(FindValueRsp* r)
	{
		node->recieved(r,srv,mtid);
	}
	
	void DHT::storeValue(StoreValueReq* r)
	{
		node->recieved(r,srv,mtid);
	}
	
	void DHT::storeValue(StoreValueRsp* r)
	{
		node->recieved(r,srv,mtid);
	}
	
	void DHT::error(ErrMsg* r)
	{}

	void DHT::portRecieved(const QString & ip,bt::Uint16 port)
	{
		Out() << "Sending ping request to " << ip << ":" << port << endl;
		PingReq* r = new PingReq(mtid,node->getOurID());
		r->setOrigin(KInetSocketAddress(ip,port));
		srv->doCall(r);
	}

}
