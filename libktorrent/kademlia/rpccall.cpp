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
#include "rpcmsg.h"
#include "rpccall.h"
#include "rpcserver.h"

namespace dht
{
	RPCCallListener::RPCCallListener() : call(0) {}
	
	RPCCallListener::~RPCCallListener() 
	{
		if (call)
		{
			call->setListener(0);
			call = 0;
		}
	}

	RPCCall::RPCCall(RPCServer* rpc,MsgBase* msg) : msg(msg),rpc(rpc),listener(0)
	{
		connect(&timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
		timer.start(20*1000,true);
	}


	RPCCall::~RPCCall()
	{
		if (listener)
			listener->call = 0;
		delete msg;
	}
	
	void RPCCall::onTimeout()
	{
		if (listener)
			listener->onTimeout(this);
		
		rpc->timedOut(msg->getMTID());
		delete msg;
		msg = 0;
	}
	
	void RPCCall::response(MsgBase* rsp)
	{
		if (listener)
			listener->onResponse(this,rsp);
	}
	
	Method RPCCall::getMsgMethod() const
	{
		if (msg)
			return msg->getMethod();
		else
			return dht::NONE;
	}
	
	void RPCCall::setListener(RPCCallListener* cl)
	{
		if (listener)
			listener->call = 0;
		
		listener = cl;
		if (listener)
			listener->call = this;
	}

}
#include "rpccall.moc"
