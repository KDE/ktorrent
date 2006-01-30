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
#ifndef BTGLOBALS_H
#define BTGLOBALS_H

#include <util/constants.h>

class QString;

namespace bt
{
	class Log;
	class Server;
	class GarbageCollector;

	Log& Out();

	class Globals
	{
	public:
		virtual ~Globals();
		
		void initLog(const QString & file);
		void initServer(Uint16 port);
		void setDebugMode(bool on) {debug_mode = on;}
		void setCriticalOperationMode(bool on) {critical_operation = on;}
		bool inCriticalOperationMode() const {return critical_operation;}
		bool isDebugModeSet() const {return debug_mode;}

		Log & getLog() {return *log;}
		Server & getServer() {return *server;}
#ifdef KT_DEBUG_GC
		GarbageCollector & getGC() {return *gc;}
#endif
		
		static Globals & instance();
		static void cleanup();
	private:
		Globals();
		
		bool debug_mode;
		bool critical_operation;
		Log* log;
		Server* server;
#ifdef KT_DEBUG_GC
		GarbageCollector* gc;
#endif
		friend Log& Out();

		static Globals* inst;
		
	};


}

#endif
