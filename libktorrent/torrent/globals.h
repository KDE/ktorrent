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
#ifndef BTGLOBALS_H
#define BTGLOBALS_H

#include <util/constants.h>
#include <ktorrent_export.h>


namespace net
{
	class PortList;
}

namespace dht
{
	class DHTBase;
}

namespace bt
{
	class Server;

	

	class KTORRENT_EXPORT Globals
	{
	public:
		virtual ~Globals();
		
		void initServer(Uint16 port);
		void shutdownServer();

		Server & getServer() {return *server;}
		dht::DHTBase & getDHT() {return *dh_table;}
		net::PortList & getPortList() {return *plist;}
				
		static Globals & instance();
		static void cleanup();
	private:
		Globals();
		
		Server* server;
		dht::DHTBase* dh_table;
		net::PortList* plist;
		
		static Globals* inst;
	};
}

#endif
