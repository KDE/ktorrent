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
#include <libutil/constants.h>
#include "key.h"

using bt::Uint32;
using bt::Uint16;

namespace dht
{
	/**
	 * @author Joris Guisson
	 *
	 * Entry in a KBucket, it basicly contains an ip_address of a node,
	 * the udp port of the node and a node_id.
	 */
	class KBucketEntry
	{
		Uint32 ip_address;
		Uint16 udp_port;
		Key node_id;
	public:
		/**
		 * Constructor, sets everything to 0.
		 * @return 
		 */
		KBucketEntry();
		
		/**
		 * Constructor, set the ip, port and key
		 * @param ip IP address
		 * @param port UDP port
		 * @param id ID of node
		 */
		KBucketEntry(Uint32 ip,Uint16 port,const Key & id);
		
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
	
	};
}

#endif
