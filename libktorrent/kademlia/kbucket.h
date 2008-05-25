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
#ifndef DHTKBUCKET_H
#define DHTKBUCKET_H

#include <qvaluelist.h>
#include <util/constants.h>
#include <ksocketaddress.h>
#include "key.h"
#include "rpccall.h"
#include "task.h"

using bt::Uint32;
using bt::Uint16;
using bt::Uint8;
using KNetwork::KInetSocketAddress;

namespace bt
{
	class File;
}

namespace dht
{
	class RPCServer;
	class KClosestNodesSearch;
	class Node;
	class Task;
	
	const Uint32 K = 8;
	const Uint32 BUCKET_MAGIC_NUMBER = 0xB0C4B0C4;
	const Uint32 BUCKET_REFRESH_INTERVAL = 15 * 60 * 1000;
//	const Uint32 BUCKET_REFRESH_INTERVAL = 120 * 1000;
	
	struct BucketHeader
	{
		Uint32 magic;
		Uint32 index;
		Uint32 num_entries;
	};
	
	/**
	 * @author Joris Guisson
	 *
	 * Entry in a KBucket, it basically contains an ip_address of a node,
	 * the udp port of the node and a node_id.
	 */
	class KBucketEntry
	{
		KInetSocketAddress addr;
		Key node_id;
		bt::TimeStamp last_responded;
		Uint32 failed_queries;
		Uint32 questionable_pings;
	public:
		/**
		 * Constructor, sets everything to 0.
		 * @return 
		 */
		KBucketEntry();
		
		/**
		 * Constructor, set the ip, port and key
		 * @param addr socket address
		 * @param id ID of node
		 */
		KBucketEntry(const KInetSocketAddress & addr,const Key & id);
		
		/**
		 * Copy constructor.
		 * @param other KBucketEntry to copy
		 * @return 
		 */
		KBucketEntry(const KBucketEntry & other);

		/// Destructor
		virtual ~KBucketEntry();

		/**
		 * Assignment operator.
		 * @param other Node to copy
		 * @return this KBucketEntry
		 */
		KBucketEntry & operator = (const KBucketEntry & other);
		
		/// Equality operator
		bool operator == (const KBucketEntry & entry) const;
		
		/// Get the socket address of the node
		const KInetSocketAddress & getAddress() const {return addr;}
		
		/// Get it's ID
		const Key & getID() const {return node_id;}
		
		/// Is this node a good node
		bool isGood() const;
		
		/// Is this node questionable (haven't heard from it in the last 15 minutes)
		bool isQuestionable() const;
		
		/// Is it a bad node. (Hasn't responded to a query
		bool isBad() const;
		
		/// Signal the entry that the peer has responded
		void hasResponded();
		
		/// A request timed out
		void requestTimeout() {failed_queries++;}
		
		/// The entry has been pinged because it is questionable
		void onPingQuestionable() {questionable_pings++;}

		/// The null entry
		static KBucketEntry null; 
	};
	
	
	/**
	 * @author Joris Guisson
	 *
	 * A KBucket is just a list of KBucketEntry objects.
	 * The list is sorted by time last seen :
	 * The first element is the least recently seen, the last
	 * the most recently seen.
	 */
	class KBucket : public RPCCallListener
	{
		Q_OBJECT
				
		Uint32 idx;
		QValueList<KBucketEntry> entries,pending_entries;
		RPCServer* srv;
		Node* node;
		QMap<RPCCall*,KBucketEntry> pending_entries_busy_pinging;
		mutable bt::TimeStamp last_modified;
		Task* refresh_task;
	public:
		KBucket(Uint32 idx,RPCServer* srv,Node* node);
		virtual ~KBucket();
		
		/**
		 * Inserts an entry into the bucket. 
		 * @param entry The entry to insert
		 */
		void insert(const KBucketEntry & entry);
		
		/// Get the least recently seen node
		const KBucketEntry & leastRecentlySeen() const {return entries[0];}
		
		/// Get the number of entries
		Uint32 getNumEntries() const {return entries.count();}
	
		/// See if this bucket contains an entry
		bool contains(const KBucketEntry & entry) const;
		
		/**
		 * Find the K closest entries to a key and store them in the KClosestNodesSearch
		 * object.
		 * @param kns The object to storre the search results
		 */
		void findKClosestNodes(KClosestNodesSearch & kns);
		
		/**
		 * A peer failed to respond
		 * @param addr Address of the peer
		 */
		bool onTimeout(const KInetSocketAddress & addr);
		
		/// Check if the bucket needs to be refreshed
		bool needsToBeRefreshed() const;
		
		/// save the bucket to a file
		void save(bt::File & fptr);
		
		/// Load the bucket from a file
		void load(bt::File & fptr,const BucketHeader & hdr);
		
		/// Update the refresh timer of the bucket
		void updateRefreshTimer();
		
		/// Set the refresh task
		void setRefreshTask(Task* t);
		
	private:
		virtual void onResponse(RPCCall* c,MsgBase* rsp);
		virtual void onTimeout(RPCCall* c);
		void pingQuestionable(const KBucketEntry & replacement_entry);
		bool replaceBadEntry(const KBucketEntry & entry);
		
	private slots:
		void onFinished(Task* t);
	};
}

#endif
