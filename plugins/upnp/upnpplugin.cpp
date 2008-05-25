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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <kpopupmenu.h>
#include <interfaces/guiinterface.h>
#include <util/fileops.h>
#include "upnpplugin.h"
#include "upnpmcastsocket.h"
#include "upnpprefpage.h"


#define NAME "UPnP"
#define AUTHOR "Joris Guisson"
#define EMAIL "joris.guisson@gmail.com"



K_EXPORT_COMPONENT_FACTORY(ktupnpplugin,KGenericFactory<kt::UPnPPlugin>("ktupnpplugin"))

namespace kt
{

	UPnPPlugin::UPnPPlugin(QObject* parent, const char* name, const QStringList& args)
	: Plugin(parent, name, args,NAME,i18n("UPnP"),AUTHOR,EMAIL,i18n("Uses UPnP to automatically forward ports on your router"),"ktupnp")
	{
		sock = 0;
		pref = 0;
	}


	UPnPPlugin::~UPnPPlugin()
	{
		delete sock;
		delete pref;
	}


	void UPnPPlugin::load()
	{
		//KIconLoader* iload = KGlobal::iconLoader();
		sock = new UPnPMCastSocket();
		pref = new UPnPPrefPage(sock);
		this->getGUI()->addPrefPage(pref);
		// load the routers list
		QString routers_file = KGlobal::dirs()->saveLocation("data","ktorrent") + "routers";
		if (bt::Exists(routers_file))
			sock->loadRouters(routers_file);
		sock->discover();
	}

	void UPnPPlugin::unload()
	{
		QString routers_file = KGlobal::dirs()->saveLocation("data","ktorrent") + "routers";
		sock->saveRouters(routers_file);
		this->getGUI()->removePrefPage(pref);
		sock->close();
		delete pref;
		pref = 0;
		delete sock;
		sock = 0;
	}
	
	void UPnPPlugin::shutdown(bt::WaitJob* job)
	{
		pref->shutdown(job);
	}
	
	bool UPnPPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}
}
#include "upnpplugin.moc"
