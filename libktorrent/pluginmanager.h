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
#ifndef KTPLUGINMANAGER_H
#define KTPLUGINMANAGER_H

#include <qptrlist.h>
#include <interfaces/plugin.h>


namespace kt
{
	class CoreInterface;
	class GUIInterface;
	/**
	 * @author Joris Guisson
	 * @brief Class to manage plugins
	 *
	 * This class manages all plugins. Plugins are stored in a list.
	 */
	class PluginManager
	{
		QPtrList<Plugin> plugins;
		CoreInterface* core;
		GUIInterface* gui;
	public:
		PluginManager(CoreInterface* core,GUIInterface* gui);
		virtual ~PluginManager();

		/**
		 * Load the list of plugins.
		 * This basicly uses KTrader to get a list of available plugins, and
		 * loads those, but does not initialize them. We will consider a plugin loaded
		 * when it's load method is called.
		 * NOTE: for now it loads all plugins
		 */
		void loadPluginList();

		/**
		 * Unload all plugins.
		 */
		void unloadAll();
	};

}

#endif
