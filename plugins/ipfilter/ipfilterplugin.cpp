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
#include <kgenericfactory.h>

#include <interfaces/coreinterface.h>
#include <interfaces/guiinterface.h>
#include <util/constants.h>
#include <torrent/ipblocklist.h>

#include <qstring.h>

#include "ipfilterplugin.h"
#include "ipfilterpluginsettings.h"
#include "antip2p.h"

using namespace bt;

K_EXPORT_COMPONENT_FACTORY(ktipfilterplugin,KGenericFactory<kt::IPFilterPlugin>("ipfilterplugin"))

namespace kt
{	
	const QString NAME = "IP Filter";
	const QString AUTHOR = "Ivan Vasic";
	const QString EMAIL = "ivasic@gmail.com";
	const QString DESCRIPTION = i18n("Filters out unwanted peers based on their IP address");

	IPFilterPlugin::IPFilterPlugin(QObject* parent, const char* name, const QStringList& args)
	: Plugin(parent, name, args,NAME,i18n("IP Filter"),AUTHOR,EMAIL,DESCRIPTION,"filter")
	{
		// setXMLFile("ktpluginui.rc");
		level1 = 0;
	}


	IPFilterPlugin::~IPFilterPlugin()
	{
		//...just in case something goes wrong...
		IPBlocklist& ipblist = IPBlocklist::instance();
		ipblist.unsetPluginInterfacePtr();
	}

	void IPFilterPlugin::load()
	{
		pref = new IPBlockingPrefPage(getCore(), this);
		getGUI()->addPrefPage(pref);
		
		if(IPBlockingPluginSettings::useLevel1())
			loadAntiP2P();
		
		//now we need to set a pointer to the IPBlocklist
		IPBlocklist& ipblist = IPBlocklist::instance();
		ipblist.setPluginInterfacePtr(this);
	}

	void IPFilterPlugin::unload()
	{
		//First unset pointer in IPBlocklist
		IPBlocklist& ipblist = IPBlocklist::instance();
		ipblist.unsetPluginInterfacePtr();
		
		getGUI()->removePrefPage(pref);
		delete pref;
		pref = 0;
		if(level1)
		{
			delete level1;
			level1 = 0;
		}
	}
	
	bool IPFilterPlugin::loadAntiP2P()
	{
		if(level1 != 0)
			return true;
		level1 = new AntiP2P();
		if(!level1->exists())
		{
			delete level1;
			level1 = 0;
			return false;
		}
		level1->loadHeader();
		return true;
	}
	
	bool IPFilterPlugin::unloadAntiP2P()
	{
		if(level1 != 0)
		{
			delete level1;
			level1 = 0;
			return true;
		}
		else
			//anything else to check?
			return true;
	}
	
	bool IPFilterPlugin::isBlockedIP(const QString& ip)
	{
		if (level1 == 0)
			return false;
		
		return level1->isBlockedIP(ip);
	}

	bool IPFilterPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}
}
