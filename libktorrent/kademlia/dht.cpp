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
#include "dht.h"
#include "node.h"
#include "rpcserver.h"
#include "rpcmsg.h"

namespace dht
{

	DHT::DHT() : node(0),srv(0)
	{
		node = new Node();
		srv = new RPCServer(this,4000);
	}


	DHT::~DHT()
	{
		delete srv;
		delete node;
	}

	void DHT::ping(PingReq* r)
	{
	}
	
	void DHT::ping(PingRsp* r)
	{
	}
	
	void DHT::findNode(FindNodeReq* r)
	{}
	
	void DHT::findNode(FindNodeRsp* r)
	{}
	
	void DHT::findValue(FindValueReq* r)
	{}
	
	void DHT::findValue(FindValueRsp* r)
	{}
	
	void DHT::storeValue(StoreValueReq* r)
	{}
	
	void DHT::storeValue(StoreValueRsp* r)
	{}
	
	void DHT::error(ErrMsg* r)
	{}

}
