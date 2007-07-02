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
#ifndef DHTTASK_H
#define DHTTASK_H

#include <qvaluelist.h>
#include "rpccall.h"
//#include "kbucket.h"

namespace KNetwork
{
	class KResolverResults;
}

namespace dht
{
	class Node;
	class Task;
	class KClosestNodesSearch;
	class KBucketEntry;
	
	const Uint32 MAX_CONCURRENT_REQS = 16;

	using KNetwork::KResolverResults;

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Performs a task on K nodes provided by a KClosestNodesSearch.
	 * This is a base class for all tasks.
	 */
	class Task : public RPCCallListener
	{
		Q_OBJECT
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
		 * object into the todo list. And call update if the task is not queued.
		 * @param kns The KClosestNodesSearch object
		 * @param queued Is the task queued
		 */
		void start(const KClosestNodesSearch & kns,bool queued);
		
		
		/**
		 *  Start the task, to be used when a task is queued.
		 */
		void start();

		/// Decrements the outstanding_reqs
		virtual void onResponse(RPCCall* c, MsgBase* rsp);
		
		/// Decrements the outstanding_reqs
		virtual void onTimeout(RPCCall* c);
		
		/**
		 * Will continue the task, this will be called every time we have
		 * rpc slots available for this task. Should be implemented by derived classes.
		 */
		virtual void update() = 0;
		
		/**
		 * A call is finished and a response was received.
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
		bool isFinished() const {return task_finished;}
		
		/// Set the task ID
		void setTaskID(bt::Uint32 tid) {task_id = tid;}
		
		/// Get the task ID
		bt::Uint32 getTaskID() const {return task_id;}
		
		/// Get the number of outstanding requests
		bt::Uint32 getNumOutstandingRequests() const {return outstanding_reqs;}
		
		bool isQueued() const {return queued;}
		
		/**
		 * Tell listeners data is ready.
		 */
		void emitDataReady();
		
		/// Kills the task
		void kill();
		
		/**
		 * Add a node to the todo list
		 * @param ip The ip or hostname of the node
		 * @param port The port
		 */
		void addDHTNode(const QString & ip,bt::Uint16 port);
		
	signals:
		/**
		 * The task is finsihed.
		 * @param t The Task
		 */
		void finished(Task* t);
		
		/**
		 * Called by the task when data is ready.
		 * Can be overrided if wanted.
		 * @param t The Task
		 */
		void dataReady(Task* t);
		
	protected:
		void done();
		
	protected slots:
		void onResolverResults(KResolverResults res);
				
	protected:	
		QValueList<KBucketEntry> visited; // nodes visited
		QValueList<KBucketEntry> todo; // nodes todo
		Node* node;
		
	private:
		RPCServer* rpc;
		bt::Uint32 outstanding_reqs;
		bt::Uint32 task_id; 
		bool task_finished;
		bool queued;
	};

}

#endif
