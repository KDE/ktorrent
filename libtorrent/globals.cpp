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

#include <libutil/log.h>
#include <libutil/error.h>
#include "globals.h"
#include "server.h"


namespace bt
{
	
	

	Globals Globals::inst;





	Globals::Globals()
	{
		debug_mode = false;
		log = new Log();
		server = 0;
	}

	Globals::~ Globals()
	{
		delete server;
		delete log;
	}

	void Globals::initLog(const QString & file)
	{
		log->setOutputFile(file);
		log->setOutputToConsole(debug_mode);
	}

	void Globals::initServer(Uint16 port)
	{
		if (server)
		{
			delete server;
			server = 0;
		}
		
		server = new Server(port);
	}

	Log & Out()
	{
		Log & lg = Globals::instance().getLog();
		lg.setOutputToConsole(Globals::instance().isDebugModeSet());
		return lg;
	}
}

