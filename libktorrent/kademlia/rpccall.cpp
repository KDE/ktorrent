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
#include "dht.h"
#include "rpcmsg.h"
#include "rpccall.h"
#include "rpcserver.h"

namespace dht
{
	RPCCallListener::RPCCallListener()
	{}
	
	RPCCallListener::~RPCCallListener() 
	{
	}

	RPCCall::RPCCall(RPCServer* rpc,MsgBase* msg,bool queued) : msg(msg),rpc(rpc),queued(queued)
	{
		connect(&timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
		if (!queued)
			timer.start(30*1000,true);
	}


	RPCCall::~RPCCall()
	{
		delete msg;
	}
	
	void RPCCall::start()
	{
		queued = false;
		timer.start(30*1000,true);
	}
	
	void RPCCall::onTimeout()
	{
		onCallTimeout(this);
		rpc->timedOut(msg->getMTID());
	}
	
	void RPCCall::response(MsgBase* rsp)
	{
		onCallResponse(this,rsp);
	}
	
	Method RPCCall::getMsgMethod() const
	{
		if (msg)
			return msg->getMethod();
		else
			return dht::NONE;
	}
	
	void RPCCall::addListener(RPCCallListener* cl)
	{
		connect(this,SIGNAL(onCallResponse( RPCCall*, MsgBase* )),cl,SLOT(onResponse( RPCCall*, MsgBase* )));
		connect(this,SIGNAL(onCallTimeout( RPCCall* )),cl,SLOT(onTimeout( RPCCall* )));
	}

}
#include "rpccall.moc"
