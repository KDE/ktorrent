/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef KTIPFILTERPLUGIN_H
#define KTIPFILTERPLUGIN_H

#include <interfaces/plugin.h>
#include <interfaces/ipblockinginterface.h>

#include "ipblockingprefpage.h"
#include "antip2p.h"

class QString;

namespace kt
{	
	class IPBlockingPrefPage;
	
	/**
	 * @author Ivan Vasic <ivasic@gmail.com>
	 * @brief IP filter plugin
	 * 
	 * This plugin will load IP ranges from specific files into KT IPBlocklist.
	 */
	class IPFilterPlugin : public Plugin, public kt::IPBlockingInterface
	{
		Q_OBJECT
	public:
		IPFilterPlugin(QObject* parent, const char* name, const QStringList& args);
		virtual ~IPFilterPlugin();

		virtual void load();
		virtual void unload();
		
		///Loads the KT format list filter
		void loadFilters();
		
		///Loads the anti-p2p filter list
		bool loadAntiP2P();
		
		///Unloads the anti-p2p filter list
		bool unloadAntiP2P();
		
		
		///Checks if IP is listed in AntiP2P filter list.
		bool isBlockedIP(const QString& ip);
		
		bool versionCheck(const QString & version) const;
	private:
		IPBlockingPrefPage* pref;
		AntiP2P* level1;
	};

}

#endif
