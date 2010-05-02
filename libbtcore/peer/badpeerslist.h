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
#ifndef BTBADPEERSLIST_H
#define BTBADPEERSLIST_H

#include <QStringList>
#include <interfaces/blocklistinterface.h>

namespace bt
{

	/**
		Blocklist to keep track of bad peers.
	*/
	class BadPeersList : public BlockListInterface
	{
	public:
		BadPeersList();
		virtual ~BadPeersList();

		virtual bool isBlockedIP(const net::Address & addr);
		virtual bool isBlockedIP(const QString & addr);
		
		/// Add a bad peer to the list
		void addBadPeer(const QString & ip);

	private:
		QStringList bad_peers;
	};

}

#endif
