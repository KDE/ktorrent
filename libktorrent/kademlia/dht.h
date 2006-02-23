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
#ifndef DHTDHT_H
#define DHTDHT_H

#include <qstring.h>
#include <util/constants.h>
#include <util/ptrmap.h>
#include "task.h"

namespace dht
{
	class Node;
	class RPCServer;
	class PingReq;
	class PingRsp;
	class FindNodeReq;
	class FindNodeRsp;
	class FindValueReq;
	class FindValueRsp;
	class StoreValueReq;
	class StoreValueRsp;
	class ErrMsg;
	class MsgBase;
	class Database;

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class DHT
	{
	public:
		DHT();
		virtual ~DHT();
		
		void ping(PingReq* r);
		void ping(PingRsp* r);
		void findNode(FindNodeReq* r);
		void findNode(FindNodeRsp* r);
		void findValue(FindValueReq* r);
		void findValue(FindValueRsp* r);
		void storeValue(StoreValueReq* r);
		void storeValue(StoreValueRsp* r);
		void error(ErrMsg* r);
		
		/**
		 * A Peer has recieved a PORT message, and uses this function to alert the DHT of it.
		 * @param ip The IP of the peer
		 * @param port The port in the PORT message
		 */
		void portRecieved(const QString & ip,bt::Uint16 port);
		
		/**
		 * Start a new Task, the dht will make sure the task 
		 * is deleted at the appropriate time.
		 * @param task A Task (must be created with new)
		 * @return The ID of the task
		 */
		Uint32 startTask(Task* task);
		
		/**
		 * Update the dht.
		 */
		void update();

	private:
		Node* node;
		RPCServer* srv;
		Database* db;
		bt::PtrMap<Uint32,Task> tasks;
		bt::Uint32 next_id;
	};

}

#endif
