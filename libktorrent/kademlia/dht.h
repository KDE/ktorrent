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
#include "key.h"

namespace bt
{
	class SHA1Hash;
}


namespace dht
{
	class Node;
	class RPCServer;
	class PingReq;
	class FindNodeReq;
	class FindValueReq;
	class StoreValueReq;
	class GetPeersReq;
	class MsgBase;
	class ErrMsg;
	class MsgBase;
	class AnnounceReq;
	class Database;
	class TaskManager;
	class Task;
	class AnnounceTask;

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class DHT
	{
	public:
		DHT();
		virtual ~DHT();
		
		void ping(PingReq* r);
		void findNode(FindNodeReq* r);
		void findValue(FindValueReq* r);
		void storeValue(StoreValueReq* r);
		void response(MsgBase* r);
		void getPeers(GetPeersReq* r);
		void announce(AnnounceReq* r);
		void error(ErrMsg* r);
		
		/**
		 * A Peer has recieved a PORT message, and uses this function to alert the DHT of it.
		 * @param ip The IP of the peer
		 * @param port The port in the PORT message
		 */
		void portRecieved(const QString & ip,bt::Uint16 port);
		
		/**
		 * Do an announce on the DHT network
		 * @param info_hash The info_hash
		 * @param port The port
		 * @return The task which handles this
		 */
		AnnounceTask* announce(const bt::SHA1Hash & info_hash,bt::Uint16 port);
		
	private:
		dht::Key genToken(GetPeersReq* r);
		
	private:
		Node* node;
		RPCServer* srv;
		Database* db;
		TaskManager* tman;
		
	};

}

#endif
