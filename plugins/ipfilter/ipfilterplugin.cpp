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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <kgenericfactory.h>
#include "ipfilterplugin.h"
#include <interfaces/coreinterface.h>
#include <interfaces/guiinterface.h>
#include <torrent/ipblocklist.h>
#include <util/constants.h>
#include <qstring.h>
#include <qfile.h>
#include "ipfilterpluginsettings.h"

using namespace bt;



K_EXPORT_COMPONENT_FACTORY(ktipfilterplugin,KGenericFactory<kt::IPFilterPlugin>("ipfilterplugin"))

namespace kt
{	
	
	Uint32 toUint32(QString& ip, bool* ok)
	{
		bool test;
		*ok = true;

		Uint32 ret = ip.section('.',0,0).toULongLong(&test);
		if(!test) *ok=false;
		ret <<= 8;
		ret |= ip.section('.',1,1).toULong(&test);
		if(!test) *ok=false;
		ret <<= 8;
		ret |= ip.section('.',2,2).toULong(&test);
		if(!test) *ok=false;
		ret <<= 8;
		ret |= ip.section('.',3,3).toULong(&test);
		if(!test) *ok=false;

		if(*ok)
		{
// 			 			Out() << "IP: " << ip << " parsed: " << ret << endl;
			return ret;
		}
		else
		{
// 			Out() << "Could not parse IP " << ip << ".  IP blocklist might not be working." << endl;
			return 0;
		}
	}
	
	const QString NAME = "ipfilterplugin";
	const QString AUTHOR = "Ivan Vasic";
	const QString EMAIL = "ivasic@gmail.com";
	const QString DESCRIPTION = i18n("KTorrent's IP filter plugin");

	IPFilterPlugin::IPFilterPlugin(QObject* parent, const char* name, const QStringList& args)
	: Plugin(parent, name, args,NAME,AUTHOR,EMAIL,DESCRIPTION)
	{
		// setXMLFile("ktpluginui.rc");
	}


	IPFilterPlugin::~IPFilterPlugin()
	{}

	void IPFilterPlugin::load()
	{
		pref = new IPBlockingPrefPage(getCore());
		getGUI()->addPrefPage(pref);
		
		pref->loadFilters();
	}

	void IPFilterPlugin::unload()
	{
		getGUI()->removePrefPage(pref);
		delete pref;
		pref = 0;
	}

}
