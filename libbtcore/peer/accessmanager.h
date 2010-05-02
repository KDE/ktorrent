/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef BTACCESSMANAGER_H
#define BTACCESSMANAGER_H

#include <QList>
#include <net/address.h>
#include <btcore_export.h>

namespace bt
{
	class BlockListInterface;
	class BadPeersList;

	/**
		@author Joris Guisson
	
		Class which determines wether or not we allow an IP to connect to us.
		It uses blocklists to do this. Blocklists should register with this class.
		By default it has one blocklist, the banned peers list.
	*/
	class BTCORE_EXPORT AccessManager
	{
		AccessManager();
	public:
		virtual ~AccessManager();

		/// Get the singleton instance
		static AccessManager & instance();
		
		/// Add a blocklist
		void addBlockList(BlockListInterface* bl);
		
		/// Remove a blocklist
		void removeBlockList(BlockListInterface* bl);
		
		/// Are we allowed to have a connection with a peer 
		bool allowed(const net::Address & addr);
			
		/// Are we allowed to have a connection with a peer
		bool allowed(const QString & addr);
		
		/// Ban a peer (i.e. add it to the banned list)
		void banPeer(const QString & addr);
	private:
		QList<BlockListInterface*> blocklists;
		BadPeersList* banned;
	};

}

#endif
