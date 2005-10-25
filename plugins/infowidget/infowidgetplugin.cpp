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
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <interfaces/guiinterface.h>
#include "infowidget.h"
#include "infowidgetplugin.h"
#include "infowidgetprefpage.h"


#define NAME "infowidgetplugin"
#define AUTHOR "Joris Guisson"
#define EMAIL "joris.guisson@gmail.com"


K_EXPORT_COMPONENT_FACTORY(ktinfowidgetplugin,KGenericFactory<kt::InfoWidgetPlugin>("ktinfowidgetplugin"))

namespace kt
{
	

	InfoWidgetPlugin::InfoWidgetPlugin(QObject* parent, const char* name, const QStringList& args)
	: Plugin(parent, name, args,NAME,AUTHOR,EMAIL,i18n("KTorrent's information widget plugin, it shows additional information about a download."))
	{
		iw = 0; pref = 0;
	}


	InfoWidgetPlugin::~InfoWidgetPlugin()
	{}


	void InfoWidgetPlugin::load()
	{
		iw = new InfoWidget(0);
		pref = new InfoWidgetPrefPage(iw);
		getGUI()->addViewListener(this);
		getGUI()->addWidgetInView(iw,kt::BELOW);
		getGUI()->addPrefPage(pref);
	}

	void InfoWidgetPlugin::unload()
	{
		getGUI()->removeViewListener(this);
		getGUI()->removePrefPage(pref);
		getGUI()->removeWidgetFromView(iw);
		delete pref;
		pref = 0;
		delete iw;
		iw = 0;
	}

	void InfoWidgetPlugin::guiUpdate()
	{
		iw->update();
	}

	void InfoWidgetPlugin::currentChanged(TorrentInterface* tc)
	{
		iw->changeTC(tc);
	}
}

#include "infowidgetplugin.moc"
