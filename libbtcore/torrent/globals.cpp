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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "globals.h"

#include <net/portlist.h>
#include <dht/dht.h>
#include <net/reverseresolver.h>
#include <utp/utpserver.h>
#include "server.h"


namespace bt
{	

	Globals* Globals::inst = 0;

	Globals::Globals()
	{
		plist = new net::PortList();
		tcp_server = 0;
		utp_server = 0;
		dh_table = new dht::DHT();
	}

	Globals::~ Globals()
	{
		// shutdown the reverse resolver thread
		net::ReverseResolver::shutdown();
		shutdownUTPServer();
		delete tcp_server;
		delete dh_table;
		delete plist;
	}
	
	Globals & Globals::instance() 
	{
		if (!inst) 
			inst = new Globals();
		return *inst;
	}
	
	void Globals::cleanup()
	{
		delete inst;
		inst = 0;
	}

	bool Globals::initTCPServer(Uint16 port)
	{
		if (tcp_server)
			shutdownTCPServer();
		
		tcp_server = new Server();
		if (!tcp_server->changePort(port))
			return false;
		
		return true;
	}
	
	void Globals::shutdownTCPServer()
	{
		if (tcp_server)
		{
			tcp_server->close();
			delete tcp_server;
			tcp_server = 0;
		}
	}

	bool Globals::initUTPServer(Uint16 port)
	{
		if (utp_server)
			shutdownUTPServer();
		
		utp_server = new utp::UTPServer();
		if (!utp_server->changePort(port))
			return false;
		
		utp_server->start();
		return true;
	}

	void Globals::shutdownUTPServer()
	{
		if (utp_server)
		{
			utp_server->stop();
			delete utp_server;
			utp_server = 0;
		}
	}

}

