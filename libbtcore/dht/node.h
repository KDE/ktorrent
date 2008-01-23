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
#ifndef DHTNODE_H
#define DHTNODE_H

#include <qobject.h>
#include "key.h"
#include "kbucket.h"


using bt::Uint8;

namespace dht
{
	class DHT;
	class MsgBase;
	class RPCServer;
	class KClosestNodesSearch;
	
	/**
	 * @author Joris Guisson
	 *
	 * A Node represents us in the kademlia network. It contains
	 * our id and 160 KBucket's.
	 * A KBucketEntry is in node i, when the difference between our id and
	 * the KBucketEntry's id is between 2 to the power i and 2 to the power i+1.
	*/
	class Node : public QObject
	{
		Q_OBJECT
	public:
		Node(RPCServer* srv,const QString & key_file);
		virtual ~Node();

		/**
		 * An RPC message was received, the node must now update
		 * the right bucket.
		 * @param dh_table The DHT
		 * @param msg The message
		 * @param srv The RPCServer to send a ping if necessary
		 */
		void recieved(DHT* dh_table,const MsgBase* msg);
		
		/// Get our own ID
		const dht::Key & getOurID() const {return our_id;}
		
		/**
		 * Find the K closest entries to a key and store them in the KClosestNodesSearch
		 * object.
		 * @param kns The object to storre the search results
		 */
		void findKClosestNodes(KClosestNodesSearch & kns);
		
		/**
		 * Increase the failed queries count of the bucket entry we sent the message to
		*/
		void onTimeout(const MsgBase* msg);
		
		/// Check if a buckets needs to be refreshed, and refresh if necesarry
		void refreshBuckets(DHT* dh_table);
		
		/// Save the routing table to a file
		void saveTable(const QString & file);
		
		/// Load the routing table from a file
		void loadTable(const QString & file);
		
		/// Get the number of entries in the routing table
		Uint32 getNumEntriesInRoutingTable() const {return num_entries;}
	private:
		Uint8 findBucket(const dht::Key & id);
		
		
		
	private:
		dht::Key our_id;
		KBucket* bucket[160];
		RPCServer* srv;
		Uint32 num_receives;
		Uint32 num_entries;
		bool delete_table;
	};

}

#endif
