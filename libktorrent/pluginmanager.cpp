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
#include <kparts/componentfactory.h>
#include "pluginmanager.h"

namespace kt
{

	PluginManager::PluginManager()
	{
		plugins.setAutoDelete(true);
	}


	PluginManager::~PluginManager()
	{}


	void PluginManager::loadPluginList()
	{
		KTrader::OfferList offers = KTrader::self()->query("KTorrent/Plugin");

		KTrader::OfferList::ConstIterator iter;
		for(iter = offers.begin(); iter != offers.end(); ++iter)
		{
			/*
			KService::Ptr service = *iter;
			int errCode = 0;
			Plugin* plugin =
					KParts::ComponentFactory::createInstanceFromService<kt::Plugin>
					(service, 0, 0, QStringList(),&errCode);
        // here we ought to check the error code.
			
			if (plugin)
			{
				guiFactory()->addClient(plugin);

				kdDebug() << "PluginDemo: Loaded plugin "
						<< plugin->name() << endl;
			}
			

			plugins.append(plugin);*/
		}
	}
}
