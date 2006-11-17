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
#include <net/portlist.h>
#include "webinterfaceprefpage.h"
#include "webinterfaceplugin.h"
#include "httpserver.h"
#include "webinterfacepluginsettings.h"

#define NAME "webinterfaceplugin"
#define AUTHOR "Diego R. Brogna"
#define EMAIL "dierbro@gmail.com"

K_EXPORT_COMPONENT_FACTORY(ktwebinterfaceplugin,KGenericFactory<kt::WebInterfacePlugin>("ktwebinterfaceplugin"))

using namespace bt;
namespace kt
{
	WebInterfacePlugin::WebInterfacePlugin(QObject* parent, const char* name, const QStringList& args)
	: Plugin(parent, name, args,NAME,AUTHOR,EMAIL,i18n("Allow to control ktorrent through browser"))
	{
		httpThread=0;
		pref=0;
	}
	WebInterfacePlugin::~WebInterfacePlugin()
	{ 
	
	}
	void WebInterfacePlugin::load()
	{

		if(!httpThread)
				delete httpThread;

		httpThread=new ServerThread(getCore());
		
		httpThread->start();

		pref = new WebInterfacePrefPage(this);
		getGUI()->addPrefPage(pref);

	}

	void WebInterfacePlugin::unload()
	{
		bt::Globals::instance().getPortList().removePort(httpThread->port(),net::TCP);
		httpThread->stop();
		httpThread->wait();
		delete httpThread;
		httpThread=0;
		getGUI()->removePrefPage(pref);
		delete pref;
		pref = 0;

	}

	void WebInterfacePlugin::preferencesUpdated()
	{
		if(httpThread->port()!=WebInterfacePluginSettings::port()){
			//stop and delete http server thread
			bt::Globals::instance().getPortList().removePort(httpThread->port(),net::TCP);
			httpThread->stop();
			httpThread->wait();
			delete httpThread;
			httpThread=0;
			//restart http server thread
			httpThread=new ServerThread(getCore());
			httpThread->start();
		}
	}

}
	
#include "webinterfaceplugin.moc"
