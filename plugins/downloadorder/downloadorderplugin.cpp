/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <kmainwindow.h>
#include <kactioncollection.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <util/fileops.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <torrent/queuemanager.h>
#include "downloadorderplugin.h"
#include "downloadorderdialog.h"
#include "downloadordermanager.h"

K_EXPORT_COMPONENT_FACTORY(ktdownloadorderplugin,KGenericFactory<kt::DownloadOrderPlugin>("ktdownloadorderplugin"))
		
using namespace bt;

namespace kt
{

	DownloadOrderPlugin::DownloadOrderPlugin(QObject* parent,const QStringList& args): Plugin(parent)
	{
		Q_UNUSED(args);
		download_order_action = new KAction(KIcon("view-sort-ascending"),i18n("File Download Order"),this);
		connect(download_order_action,SIGNAL(triggered()),this,SLOT(showDownloadOrderDialog()));
		actionCollection()->addAction("download_order",download_order_action);
		setXMLFile("ktdownloadorderpluginui.rc");
		managers.setAutoDelete(true);
	}


	DownloadOrderPlugin::~DownloadOrderPlugin()
	{
	}


	bool DownloadOrderPlugin::versionCheck(const QString& version) const
	{
		return version == KT_VERSION_MACRO;
	}

	void DownloadOrderPlugin::load()
	{
		getGUI()->addViewListener(this);
		connect(getCore(),SIGNAL(torrentAdded(bt::TorrentInterface*)),this,SLOT(torrentAdded(bt::TorrentInterface*)));
		connect(getCore(),SIGNAL(torrentRemoved(bt::TorrentInterface*)),this,SLOT(torrentRemoved(bt::TorrentInterface*)));
		currentTorrentChanged(getGUI()->getCurrentTorrent());
		
		kt::QueueManager* qman = getCore()->getQueueManager();
		for (kt::QueueManager::iterator i = qman->begin();i != qman->end();i++)
			torrentAdded(*i);
	}

	void DownloadOrderPlugin::unload()
	{
		getGUI()->removeViewListener(this);
		disconnect(getCore(),SIGNAL(torrentAdded(bt::TorrentInterface*)),this,SLOT(torrentAdded(bt::TorrentInterface*)));
		disconnect(getCore(),SIGNAL(torrentRemoved(bt::TorrentInterface*)),this,SLOT(torrentRemoved(bt::TorrentInterface*)));
		managers.clear();
	}

	void DownloadOrderPlugin::showDownloadOrderDialog()
	{
		bt::TorrentInterface* tor = getGUI()->getCurrentTorrent();
		if (!tor || !tor->getStats().multi_file_torrent)
			return;
		
		DownloadOrderDialog dlg(this,tor,getGUI()->getMainWindow());
		dlg.exec();
	}
	
	void DownloadOrderPlugin::currentTorrentChanged(bt::TorrentInterface* tc)
	{
		download_order_action->setEnabled(tc && tc->getStats().multi_file_torrent);
	}
	
	DownloadOrderManager* DownloadOrderPlugin::manager(bt::TorrentInterface* tc)
	{
		return managers.find(tc);
	}
		
	DownloadOrderManager* DownloadOrderPlugin::createManager(bt::TorrentInterface* tc)
	{
		DownloadOrderManager* m = manager(tc);
		if (m)
			return m;
		
		m = new DownloadOrderManager(tc);
		managers.insert(tc,m);
		return m;
	}
	
	void DownloadOrderPlugin::destroyManager(bt::TorrentInterface* tc)
	{
		managers.erase(tc);
	}
	
	void DownloadOrderPlugin::torrentAdded(bt::TorrentInterface* tc)
	{
		if (bt::Exists(tc->getTorDir() + "download_order"))
		{
			DownloadOrderManager* m = createManager(tc);
			m->load();
			m->update();
			for (Uint32 i = 0;i < tc->getNumFiles();i++)
			{
				TorrentFileInterface & tf = tc->getTorrentFile(i);
				connect(&tf,SIGNAL(downloadPercentageChanged()),m,SLOT(update()));
			}
		}
	}
	
	void DownloadOrderPlugin::torrentRemoved(bt::TorrentInterface* tc)
	{
		managers.erase(tc);
	}
}
