/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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
#include <util/log.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>
#include <settings.h>

#include "infowidgetplugin.h"
#include "iwprefpage.h"
#include "statustab.h"
#include "fileview.h"
#include "chunkdownloadview.h"
#include "peerview.h"
#include "trackerview.h"		
#include "infowidgetpluginsettings.h"
#include "monitor.h"


#define NAME "Info Widget"
#define AUTHOR "Joris Guisson"
#define EMAIL "joris.guisson@gmail.com"


K_EXPORT_COMPONENT_FACTORY(ktinfowidgetplugin,KGenericFactory<kt::InfoWidgetPlugin>("ktinfowidgetplugin"))
		
using namespace bt;

namespace kt
{
	

	InfoWidgetPlugin::InfoWidgetPlugin(QObject* parent,const QStringList& )
	: Plugin(parent,NAME,i18n("Info Widget"),AUTHOR,EMAIL,
			 i18n("Shows additional information about a download. Like which chunks have been downloaded, how many seeders and leechers ..."),
			 "ktinfowidget")
	{
		pref = 0;
		peer_view = 0;
		cd_view = 0;
		tracker_view = 0;
		file_view = 0;
		status_tab = 0;
		monitor = 0;
	}


	InfoWidgetPlugin::~InfoWidgetPlugin()
	{}


	void InfoWidgetPlugin::load()
	{
		connect(getCore(),SIGNAL(settingsChanged()),this,SLOT(applySettings()));
		
		status_tab = new StatusTab(0);
		file_view = new FileView(0);
		connect(getCore(),SIGNAL(torrentRemoved(bt::TorrentInterface*)),file_view,SLOT(onTorrentRemoved(bt::TorrentInterface*)));

		pref = new IWPrefPage(0);
		getGUI()->addViewListener(this);
		getGUI()->addToolWidget(status_tab,"dialog-information",i18n("Status"),GUIInterface::DOCK_BOTTOM);
		getGUI()->addToolWidget(file_view,"folder",i18n("Files"),GUIInterface::DOCK_BOTTOM);
	
		applySettings();
				
		getGUI()->addPrefPage(pref);
		currentTorrentChanged(const_cast<bt::TorrentInterface*>(getGUI()->getCurrentTorrent()));
	}

	void InfoWidgetPlugin::unload()
	{
		disconnect(getCore(),SIGNAL(settingsChanged()),this,SLOT(applySettings()));
		disconnect(getCore(),SIGNAL(torrentRemoved(bt::TorrentInterface*)),file_view,SLOT(onTorrentRemoved(bt::TorrentInterface*)));
		if (cd_view)
			cd_view->saveState(KGlobal::config()); 
		if (peer_view)
			peer_view->saveState(KGlobal::config());
		if (file_view)
			file_view->saveState(KGlobal::config());
		KGlobal::config()->sync();
		
		getGUI()->removeViewListener(this);
		getGUI()->removePrefPage(pref);
		getGUI()->removeToolWidget(status_tab);
		getGUI()->removeToolWidget(file_view);
		if (cd_view)
			getGUI()->removeToolWidget(cd_view);
		if (tracker_view)
			getGUI()->removeToolWidget(tracker_view); 
		if (peer_view)
			getGUI()->removeToolWidget(peer_view);

		delete monitor;
		monitor = 0;
		delete status_tab;
		status_tab = 0;
		delete file_view;
		file_view = 0;
		delete cd_view;
		cd_view = 0;
		delete peer_view;
		peer_view = 0;
		delete tracker_view;
		tracker_view = 0;
		pref = 0;
	}

	void InfoWidgetPlugin::guiUpdate()
	{
		if (status_tab && status_tab->isVisible())
			status_tab->update();
		
		if (file_view && file_view->isVisible())
			file_view->update();
		
		if (peer_view && peer_view->isVisible())
			peer_view->update();
		
		if (cd_view && cd_view->isVisible())
			cd_view->update();
		
		if (tracker_view && tracker_view->isVisible())
			tracker_view->update();
	}

	void InfoWidgetPlugin::currentTorrentChanged(bt::TorrentInterface* tc)
	{
		if (status_tab)
			status_tab->changeTC(tc);
		if (file_view)
			file_view->changeTC(tc,KGlobal::config());
		if (cd_view)
			cd_view->changeTC(tc);
		if (tracker_view)
			tracker_view->changeTC(tc);
		
		if (peer_view)
			peer_view->setEnabled(tc != 0);

		createMonitor(tc);
	}
	
	bool InfoWidgetPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}

	void InfoWidgetPlugin::applySettings()
	{
		showPeerView( InfoWidgetPluginSettings::showPeerView() );
		showChunkView( InfoWidgetPluginSettings::showChunkView() );
		showTrackerView( InfoWidgetPluginSettings::showTrackersView() );
		file_view->setShowListOfFiles( Settings::useFileList(),KGlobal::config());
	}
	
	void InfoWidgetPlugin::showPeerView(bool show)
	{
		bt::TorrentInterface* tc = const_cast<bt::TorrentInterface*>(getGUI()->getCurrentTorrent());
		
		if (show && !peer_view)
		{
			peer_view = new PeerView(0);
			getGUI()->addToolWidget(peer_view,"system-users",i18n("Peers"),GUIInterface::DOCK_BOTTOM);
			peer_view->loadState(KGlobal::config());
			createMonitor(tc);
		}
		else if (!show && peer_view)
		{
			peer_view->saveState(KGlobal::config());
			getGUI()->removeToolWidget(peer_view);
			delete peer_view; peer_view = 0;
			createMonitor(tc);
		}
	}
	
	void InfoWidgetPlugin::showChunkView(bool show)
	{
		bt::TorrentInterface* tc = const_cast<bt::TorrentInterface*>(getGUI()->getCurrentTorrent());
		
		if (show && !cd_view)
		{
			cd_view = new ChunkDownloadView(0);
			getGUI()->addToolWidget(cd_view,"ktplugins",i18n("Chunks"),GUIInterface::DOCK_BOTTOM);
			
			cd_view->loadState(KGlobal::config());
			cd_view->changeTC(tc);
			createMonitor(tc);	
		}
		else if (!show && cd_view)
		{
			cd_view->saveState(KGlobal::config());
			getGUI()->removeToolWidget(cd_view);
			delete cd_view; cd_view = 0;
			createMonitor(tc);
		}
	}
	
	void InfoWidgetPlugin::showTrackerView(bool show)
	{
		if (show && !tracker_view)
		{
			tracker_view = new TrackerView(0);
			getGUI()->addToolWidget(tracker_view,"network-server",i18n("Trackers"),
					GUIInterface::DOCK_BOTTOM);
			tracker_view->changeTC(const_cast<bt::TorrentInterface*>(getGUI()->getCurrentTorrent()));
		}
		else if (!show && tracker_view)
		{
			getGUI()->removeToolWidget(tracker_view);
			delete tracker_view; tracker_view = 0;
		}
	}
	
	void InfoWidgetPlugin::createMonitor(bt::TorrentInterface* tc)
	{	
		if (monitor)
		{
			delete monitor; 
			monitor = 0;
		}
			
		if (peer_view)
			peer_view->removeAll();
		if (cd_view)
			cd_view->removeAll();
			
		if (tc && peer_view || cd_view)
			monitor = new Monitor(tc,peer_view,cd_view);
	}
}

#include "infowidgetplugin.moc"
