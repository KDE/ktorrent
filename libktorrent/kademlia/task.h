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
	class Node;
	class Task;
	class KClosestNodesSearch;
	
	const Uint32 MAX_CONCURRENT_REQS = 8;
	
	/**
	 * Classes who which to be informed of the status of tasks, shoul derive from
	 * this class.
	 */
	class TaskListener
	{
		Task* task;
	public:
		TaskListener();
		virtual ~TaskListener();
		
		/**
		 * The task is finsihed.
		 * @param t The Task
		 */
		virtual void onFinished(Task* t) = 0;
		
		/**
		 * Called by the task when data is ready.
		 * Can be overrided if wanted.
		 * @param t The Task
		 */
		virtual void onDataReady(Task* t);
		
		/**
		 * Called by the task it is about to be deleted.
		 */
		virtual void onDestroyed(Task* t) = 0;
		
		friend class Task;
	};

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
		 * @param node The node
		 */
		Task(RPCServer* rpc,Node* node);
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
		
		/// Is the task finished
		bool isFinished() const {return finished;}
		
		/// Set the task listener
		void setListener(TaskListener* tl);
		
		/// Set the task ID
		void setTaskID(bt::Uint32 tid) {task_id = tid;}
		
		/// Get the task ID
		bt::Uint32 getTaskID() const {return task_id;}
		
		/// Get the number of outstanding requests
		bt::Uint32 getNumOutstandingRequests() const {return outstanding_reqs;}
		
		/**
		 * Tell listeners data is ready.
		 */
		void emitDataReady();
	protected:
		void done();
				
	protected:	
		QValueList<KBucketEntry> visited; // nodes visited
		QValueList<KBucketEntry> todo; // nodes todo
		Node* node;
		
	private:
		RPCServer* rpc;
		bt::Uint32 outstanding_reqs;
		TaskListener* lst;
		bt::Uint32 task_id; 
		bool finished;
	};

}

#endif
