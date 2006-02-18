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
#include "task.h"
#include "kclosestnodessearch.h"
#include "rpcserver.h"

namespace dht
{

	Task::Task(RPCServer* rpc) : rpc(rpc),outstanding_reqs(0)
	{
		
	}


	Task::~Task()
	{}
	
	void Task::start(const KClosestNodesSearch & kns)
	{
		// fill the todo list
		for (KClosestNodesSearch::CItr i = kns.begin(); i != kns.end();i++)
			todo.append(i->second);
		update();
	}


	void Task::onResponse(RPCCall* c, MsgBase* rsp)
	{
		if (outstanding_reqs > 0)
			outstanding_reqs--;
		
		callFinished(c,rsp);
		
		if (canDoRequest() && !isFinished())
			update(); 
	}

	void Task::onTimeout(RPCCall* c)
	{
		if (outstanding_reqs > 0)
			outstanding_reqs--;
		
		callTimeout(c);
		
		if (canDoRequest() && !isFinished())
			update(); 
	}
	
	bool Task::rpcCall(MsgBase* req)
	{
		if (!canDoRequest())
			return false;
		
		RPCCall* c = rpc->doCall(req);
		c->setListener(this);
		outstanding_reqs++;
		return true;
	}
	

}
