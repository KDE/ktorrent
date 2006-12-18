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

class QString;

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
	class Log;
	class Server;

	

	class Globals
	{
	public:
		virtual ~Globals();
		
		void initLog(const QString & file);
		void initServer(Uint16 port);
		void setDebugMode(bool on) {debug_mode = on;}
		bool isDebugModeSet() const {return debug_mode;}
		void shutdownServer();

		Log & getLog(unsigned int arg);
		Server & getServer() {return *server;}
		dht::DHTBase & getDHT() {return *dh_table;}
		net::PortList & getPortList() {return *plist;}
				
		static Globals & instance();
		static void cleanup();
	private:
		Globals();
		
		bool debug_mode;
		Log* log;
		Server* server;
		dht::DHTBase* dh_table;
		net::PortList* plist;
		
		friend Log& Out(unsigned int arg);

		static Globals* inst;
		
	};
}

#endif
