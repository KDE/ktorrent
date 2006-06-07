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
#include "task.h"
#include "kclosestnodessearch.h"
#include "rpcserver.h"
#include "kbucket.h"

namespace dht
{
	TaskListener::TaskListener() : task(0)
	{}
			
	TaskListener::~TaskListener()
	{
		if (task)
			task->setListener(0);
	}
	
	void TaskListener::onDataReady(Task*) 
	{}

	Task::Task(RPCServer* rpc,Node* node) 
		: node(node),rpc(rpc),outstanding_reqs(0),
		lst(0),finished(false),queued(queued)
	{
		
	}


	Task::~Task()
	{
		if (lst)
		{
			lst->onDestroyed(this);
			lst = 0;
		}
	}
	
	void Task::setListener(TaskListener* tl)
	{
		lst = tl;
		if (lst)
			lst->task = this;
	}
	
	void Task::start(const KClosestNodesSearch & kns,bool queued)
	{
		// fill the todo list
		for (KClosestNodesSearch::CItr i = kns.begin(); i != kns.end();i++)
			todo.append(i->second);
		this->queued = queued;
		if (!queued)
			update();
	}
	
	void Task::start()
	{
		if (queued)
		{
			queued = false;
			update();
		}
	}


	void Task::onResponse(RPCCall* c, MsgBase* rsp)
	{
		if (outstanding_reqs > 0)
			outstanding_reqs--;
		
		if (!isFinished())
		{
			callFinished(c,rsp);
		
			if (canDoRequest() && !isFinished())
				update(); 
		}
	}

	void Task::onTimeout(RPCCall* c)
	{
		if (outstanding_reqs > 0)
			outstanding_reqs--;
		
		if (!isFinished())
		{
			callTimeout(c);
			
			if (canDoRequest() && !isFinished())
				update(); 
		}
	}
	
	bool Task::rpcCall(MsgBase* req)
	{
		if (!canDoRequest())
			return false;
		
		RPCCall* c = rpc->doCall(req);
		c->addListener(this);
		outstanding_reqs++;
		return true;
	}
	
	void Task::done()
	{
		finished = true;
		if (lst)
			lst->onFinished(this);
	}
	
	void Task::emitDataReady()
	{
		if (lst)
			lst->onDataReady(this);
	}
	
	void Task::kill()
	{
		finished = true;
		if (lst)
			lst->onFinished(this);
	}

}
