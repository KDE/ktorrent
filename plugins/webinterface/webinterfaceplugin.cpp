  /***************************************************************************
 *   Copyright (C) 2006 by Diego R. Brogna                                 *
 *   dierbro@gmail.com                                               	   *
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
#include <kglobal.h>

#include <util/log.h>
#include <interfaces/coreinterface.h>
#include <interfaces/guiinterface.h>
#include <interfaces/torrentinterface.h>
#include <torrent/globals.h>
#include <net/portlist.h>
#include "webinterfaceprefpage.h"
#include "webinterfaceplugin.h"
#include "httpserver.h"
#include "webinterfacepluginsettings.h"

#define NAME "Web Interface"
#define AUTHOR "Diego R. Brogna"
#define EMAIL "dierbro@gmail.com"

K_EXPORT_COMPONENT_FACTORY(ktwebinterfaceplugin,KGenericFactory<kt::WebInterfacePlugin>("ktwebinterfaceplugin"))

using namespace bt;
namespace kt
{
	WebInterfacePlugin::WebInterfacePlugin(QObject* parent, const char* name, const QStringList& args)
	: Plugin(parent, name, args,NAME,i18n("Web Interface"),AUTHOR,EMAIL,i18n("Allow to control ktorrent through browser"),"toggle_log")
	{
		http_server = 0;
		pref=0;
	}
	
	WebInterfacePlugin::~WebInterfacePlugin()
	{ 
	
	}
	
	void WebInterfacePlugin::load()
	{
		initServer();
		
		pref = new WebInterfacePrefPage(this);
		getGUI()->addPrefPage(pref);

	}

	void WebInterfacePlugin::unload()
	{
		if (http_server)
		{
			bt::Globals::instance().getPortList().removePort(http_server->port(),net::TCP);
			delete http_server;
			http_server = 0;
		}
		
		getGUI()->removePrefPage(pref);
		delete pref;
		pref = 0;
	}
	
	void WebInterfacePlugin::initServer()
	{
		bt::Uint16 port = WebInterfacePluginSettings::port();
		bt::Uint16 i = 0;
				
		while (i < 10)
		{
			http_server = new HttpServer(getCore(),port + i);
			if (!http_server->ok())
			{
				delete http_server;
				http_server = 0;
			}
			else
				break;
			i++;
		}

		if (http_server)
		{
			if(WebInterfacePluginSettings::forward())
				bt::Globals::instance().getPortList().addNewPort(http_server->port(),net::TCP,true);
			Out(SYS_WEB|LOG_ALL) << "Web server listen on port "<< http_server->port() << endl;
		}
		else
		{
			Out(SYS_WEB|LOG_ALL) << "Cannot bind to port " << port <<" or the 10 following ports. WebInterface plugin cannot be loaded." << endl;
			return;
		}
	}

	void WebInterfacePlugin::preferencesUpdated()
	{
		if( http_server && http_server->port() != WebInterfacePluginSettings::port())
		{
			//stop and delete http server 
			bt::Globals::instance().getPortList().removePort(http_server->port(),net::TCP);
			delete http_server;
			http_server = 0;
			// reinitialize server
			initServer();
		}
	}

	bool WebInterfacePlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}
}
	
#include "webinterfaceplugin.moc"
