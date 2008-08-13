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
#ifndef DHTKCLOSESTNODESSEARCH_H
#define DHTKCLOSESTNODESSEARCH_H

#include <map>
#include "key.h"
#include "kbucket.h"

namespace dht
{

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Class used to store the search results during a K closests nodes search
	 * Note: we use a std::map because of lack of functionality in QMap
	*/
	class KClosestNodesSearch
	{
		dht::Key key;
		std::map<dht::Key,KBucketEntry> emap;
		Uint32 max_entries;
	public:
		/**
		 * Constructor sets the key to compare with
		 * @param key The key to compare with
		 * @param max_entries The maximum number of entries can be in the map
		 * @return 
		 */
		KClosestNodesSearch(const dht::Key & key,Uint32 max_entries);
		virtual ~KClosestNodesSearch();

		typedef std::map<dht::Key,KBucketEntry>::iterator Itr;
		typedef std::map<dht::Key,KBucketEntry>::const_iterator CItr;
		
		Itr begin() {return emap.begin();}
		Itr end() {return emap.end();}
		
		CItr begin() const {return emap.begin();}
		CItr end() const {return emap.end();}
		
		/// Get the target key of the search3
		const dht::Key & getSearchTarget() const {return key;}
		
		/// Get the number of entries.
		bt::Uint32 getNumEntries() const {return emap.size();}
		
		/**
		 * Try to insert an entry. 
		 * @param e The entry
		 */
		void tryInsert(const KBucketEntry & e);
		
		/**
		 * Pack the search results in a PackedNodeContainer.
		 * @param cnt Place to store IPv6 nodes
		 */
		void pack(PackedNodeContainer* cnt);
	};

}

#endif
