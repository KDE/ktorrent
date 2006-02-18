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
#ifndef DHTTASK_H
#define DHTTASK_H

#include <qvaluelist.h>
#include "rpccall.h"
#include "kbucket.h"

namespace dht
{
	class KClosestNodesSearch;
	
	const Uint32 MAX_CONCURRENT_REQS = 8;

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Performs a task on K nodes provided by a KClosestNodesSearch.
	 * This is a base class for all tasks.
	 */
	class Task : public RPCCallListener
	{
	public:
		/**
		 * Create a task. 
		 * @param rpc The RPC server to do RPC calls
		 */
		Task(RPCServer* rpc);
		virtual ~Task();
		
		/**
		 * This will copy the results from the KClosestNodesSearch
		 * object into the todo list. And call update.
		 * @param kns The KClosestNodesSearch object
		 */
		void start(const KClosestNodesSearch & kns);

		/// Decrements the outstanding_reqs
		virtual void onResponse(RPCCall* c, MsgBase* rsp);
		
		/// Decrements the outstanding_reqs
		virtual void onTimeout(RPCCall* c);
		
		/**
		 * Will continue the task, this will be called everytime we have
		 * rpc slots available for this task. Should be implemented by derived classes.
		 */
		virtual void update() = 0;
		
		/**
		 * A call is finsihed and a response was recieved.
		 * @param c The call
		 * @param rsp The response
		 */
		virtual void callFinished(RPCCall* c, MsgBase* rsp) = 0;
		
		/**
		 * A call timedout
		 * @param c The call
		 */
		virtual void callTimeout(RPCCall* c) = 0;
		
		/**
		 * Do a call to the rpc server, increments the outstanding_reqs variable.
		 * @param req THe request to send
		 * @return true if call was made, false if not
		 */
		bool rpcCall(MsgBase* req);
		
		/// See if we can do a request
		bool canDoRequest() const {return outstanding_reqs < MAX_CONCURRENT_REQS;}
		
		/// The task is done when the todo list is empty and there are no outstanding_reqs
		bool isFinished() const {return todo.empty() && outstanding_reqs == 0;}
	protected:	
		QValueList<KBucketEntry> visited; // nodes visited
		QValueList<KBucketEntry> todo; // nodes todo
		
	private:
		RPCServer* rpc;
		bt::Uint32 outstanding_reqs;
	};

}

#endif
