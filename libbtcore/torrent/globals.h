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
#include <btcore_export.h>

namespace utp
{
	class UTPServer;
}

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

	

	class BTCORE_EXPORT Globals
	{
	public:
		virtual ~Globals();
		
		bool initTCPServer(Uint16 port);
		void shutdownTCPServer();
		
		bool initUTPServer(Uint16 port);
		void shutdownUTPServer();
		
		bool isUTPEnabled() const {return utp_server != 0;}
		bool isTCPEnabled() const {return tcp_server != 0;}

		Server & getTCPServer() {return *tcp_server;}
		dht::DHTBase & getDHT() {return *dh_table;}
		net::PortList & getPortList() {return *plist;}
		utp::UTPServer & getUTPServer() {return *utp_server;}
				
		static Globals & instance();
		static void cleanup();
	private:
		Globals();
		
		Server* tcp_server;
		dht::DHTBase* dh_table;
		net::PortList* plist;
		utp::UTPServer* utp_server;
		
		static Globals* inst;
	};
}

#endif
