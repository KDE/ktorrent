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
#include <klocale.h>
#include "log.h"
#include "logsystemmanager.h"

namespace bt
{
	LogSystemManager LogSystemManager::self;

	LogSystemManager::LogSystemManager()
			: QObject()
	{
		// register default systems
		registerSystem(i18n("General"),SYS_GEN);
		registerSystem(i18n("Connections"),SYS_CON);
		registerSystem(i18n("Tracker"),SYS_TRK);
		registerSystem(i18n("DHT"),SYS_DHT);
		registerSystem(i18n("Disk Input/Output"),SYS_DIO);
		registerSystem(i18n("UTP"),SYS_UTP);
	}


	LogSystemManager::~LogSystemManager()
	{
	}
	
	void LogSystemManager::registerSystem(const QString & name,Uint32 id)
	{
		systems.insert(name,id);
		registered(name);
	}
		
	void LogSystemManager::unregisterSystem(const QString & name)
	{
		if (systems.remove(name))
			unregisted(name);
	}
	
	Uint32 LogSystemManager::systemID(const QString & name)
	{
		iterator i = systems.find(name);
		if (i == systems.end())
			return 0;
		else
			return i.value();
	}


}
