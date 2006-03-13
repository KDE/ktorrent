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
#ifndef DHTKBUCKET_H
#define DHTKBUCKET_H

#include <qvaluelist.h>
#include <util/constants.h>
#include <ksocketaddress.h>
#include "key.h"

using bt::Uint32;
using bt::Uint16;
using bt::Uint8;
using KNetwork::KInetSocketAddress;

namespace dht
{
	class KClosestNodesSearch;
	
	const Uint32 K = 8;
	
	/**
	 * @author Joris Guisson
	 *
	 * Entry in a KBucket, it basicly contains an ip_address of a node,
	 * the udp port of the node and a node_id.
	 */
	class KBucketEntry
	{
		KInetSocketAddress addr;
		Key node_id;
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
		
		const KInetSocketAddress & getAddress() const {return addr;}
		const Key & getID() const {return node_id;}
	};
	
	
	/**
	 * @author Joris Guisson
	 *
	 * A KBucket is just a list of KBucketEntry objects.
	 * The list is sorted by time last seen :
	 * The first element is the least recently seen, the last
	 * the most recently seen.
	 */
	class KBucket
	{
		QValueList<KBucketEntry> entries;
	public:
		KBucket();
		virtual ~KBucket();
		
		/**
		 * Inserts an entry into the bucket. Only works when there is room in
		 * the bucket (only K entries allowed)
		 * @param entry The entry to insert
		 * @param force Force if bucket is full (remove the first one)
		 * @return true if the entry was inserted
		 */
		bool insert(const KBucketEntry & entry,bool force = false);
		
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
	};
}

#endif
