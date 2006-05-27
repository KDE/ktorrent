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
#include <interfaces/torrentinterface.h>
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
	: Plugin(parent, name, args,NAME,AUTHOR,EMAIL,i18n("Shows additional information about a download. Like which chunks have been downloaded, how many seeders and leechers ..."))
	{
		iw = 0; pref = 0;
	}


	InfoWidgetPlugin::~InfoWidgetPlugin()
	{}


	void InfoWidgetPlugin::load()
	{
		iw = new InfoWidget(0);
		iw_seed = new InfoWidget(true, 0);
		pref = new InfoWidgetPrefPage(iw);
		getGUI()->addViewListener(this);
		getGUI()->addWidgetInView(iw,kt::BELOW);
		getGUI()->addWidgetInSeedView(iw_seed, kt::BELOW);
		getGUI()->addPrefPage(pref);
		iw->changeTC(const_cast<kt::TorrentInterface*>(getGUI()->getCurrentTorrent()));
	}

	void InfoWidgetPlugin::unload()
	{
		getGUI()->removeViewListener(this);
		getGUI()->removePrefPage(pref);
		getGUI()->removeWidgetFromView(iw);
		getGUI()->removeWidgetFromSeedView(iw_seed);
		delete pref;
		pref = 0;
		delete iw;
		iw = 0;
		delete iw_seed;
		iw_seed = 0;
	}

	void InfoWidgetPlugin::guiUpdate()
	{
		if(getGUI()->getCurrentPanel() == DOWNLOAD_VIEW)
			iw->update();
		
		if(getGUI()->getCurrentPanel() == SEED_VIEW)
			iw_seed->update();
	}

	void InfoWidgetPlugin::currentDownloadChanged(TorrentInterface* tc)
	{
		if(getGUI()->getCurrentPanel() == DOWNLOAD_VIEW)
			iw->changeTC(tc);
	}
	
	void InfoWidgetPlugin::currentSeedChanged(TorrentInterface* tc)
	{
		if(getGUI()->getCurrentPanel() == SEED_VIEW)
			iw_seed->changeTC(tc);		
	}
}

#include "infowidgetplugin.moc"
