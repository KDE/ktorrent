/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
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
#include <kicon.h>
#include <klocale.h>
#include <groups/group.h>
#include <groups/groupmanager.h>
#include <interfaces/torrentinterface.h>
#include <util/log.h>
#include "viewmenu.h"
#include "view.h"


using namespace bt;

namespace kt
{
	ViewMenu::ViewMenu(GroupManager* gman,View* parent) : KMenu(parent),view(parent),gman(gman)
	{
		start_torrent = addAction(KIcon("ktstart"),i18n("Start"),parent,SLOT(startTorrents()));
		stop_torrent = addAction(KIcon("ktstop"),i18n("Stop"),parent,SLOT(stopTorrents()));
		remove_torrent = addAction(KIcon("ktremove"),i18n("Remove Torrent"),parent,SLOT(removeTorrents()));
		remove_torrent_and_data = addAction(KIcon("ktremove"),i18n("Remove Torrent and Data"),parent,SLOT(removeTorrentsAndData()));
		queue_torrent = addAction(i18n("Enqueue/Dequeue"),parent,SLOT(queueTorrents()));
		addSeparator();
		add_peers = addAction(KIcon("list-add"),i18n("Add Peers"),parent,SLOT(addPeers()));
		peer_sources_menu = addMenu(i18n("Additional Peer Sources"));
		dht_enabled = peer_sources_menu->addAction(i18n("DHT"),parent,SLOT(toggleDHT()));
		dht_enabled->setCheckable(true);
		pex_enabled = peer_sources_menu->addAction(i18n("Peer Exchange"),parent,SLOT(togglePEX()));
		pex_enabled->setCheckable(true);
		addSeparator();
		manual_announce = addAction(i18n("Manual Announce"),parent,SLOT(manualAnnounce()));
		scrape = addAction(i18n("Scrape"),parent,SLOT(scrape()));
		preview  = addAction(KIcon("document-open"),i18n("Preview"),parent,SLOT(previewTorrents()));
		addSeparator();
		dirs_menu = addMenu(KIcon("folder-open"),i18n("Open Directory"));
		data_dir = dirs_menu->addAction(i18n("Data Directory"),parent,SLOT(openDataDir()));
		tor_dir = dirs_menu->addAction(i18n("Temporary Directory"),parent,SLOT(openTorDir()));
		addSeparator();
		remove_from_group = addAction(i18n("Remove from Group"),parent,SLOT(removeFromGroup()));
		add_to_group = addMenu(i18n("Add to Group"));
		addSeparator();
		check_data = addAction(i18n("Check Data"),parent,SLOT(checkData()));
		speed_limits = addAction(i18n("Speed Limits"),parent,SLOT(speedLimitsDlg()));

		connect(add_to_group,SIGNAL(triggered(QAction * )),this,SLOT(groupsMenuItemTriggered(QAction*)));
	}

	ViewMenu::~ViewMenu()
	{
	}
		
	void ViewMenu::show(const QPoint & pos)
	{
		bool en_start = false;
		bool en_stop = false;
		bool en_remove = false;
		bool en_prev = false;
		bool en_announce = false;
		bool en_add_peer = false;
		bool en_dirs = false;
		bool en_peer_sources = false;
		bool dummy = false;

		QList<bt::TorrentInterface*> sel;
		view->getSelection(sel);
		foreach (bt::TorrentInterface* tc,sel)
		{
			const TorrentStats & s = tc->getStats();
			
			if (tc->readyForPreview() && !s.multi_file_torrent)
				en_prev = true;
			
			if (!tc->isCheckingData(dummy))
				en_remove = true;
			
			if (!s.running)
			{
				if (!tc->isCheckingData(dummy))
				{
					en_start = true;
				}
			}
			else
			{
				if (!tc->isCheckingData(dummy))
				{
					en_stop = true;
					if (tc->announceAllowed())
						en_announce = true;
				}
			}
			
			if (!s.priv_torrent && !tc->isCheckingData(dummy))
			{
				en_add_peer = true;
				en_peer_sources = true;
			}
		}

		en_add_peer = en_add_peer && en_stop;

		start_torrent->setEnabled(en_start);
		stop_torrent->setEnabled(en_stop);
		remove_torrent->setEnabled(en_remove);
		remove_torrent_and_data->setEnabled(en_remove);
		preview->setEnabled(en_prev);
		add_peers->setEnabled(en_add_peer);
		manual_announce->setEnabled(en_announce);
		scrape->setEnabled(sel.count() > 0);
		queue_torrent->setEnabled(en_remove);

		const kt::Group* current_group = view->getGroup();
		remove_from_group->setEnabled(current_group && !current_group->isStandardGroup());
		updateGroupsMenu();
		add_to_group->setEnabled(gman->count() > 0);

		if (sel.count() == 1)
		{
			//enable directories
			en_dirs = true;
			
			TorrentInterface* tc = sel.front();
			// no data check when we are preallocating diskspace
			check_data->setEnabled(tc->getStats().status != bt::ALLOCATING_DISKSPACE && !tc->isCheckingData(dummy));
			
			//enable additional peer sources if torrent is not private
			peer_sources_menu->setEnabled(en_peer_sources);
			
			if (en_peer_sources)
			{
				dht_enabled->setChecked(tc->isFeatureEnabled(bt::DHT_FEATURE));
				pex_enabled->setChecked(tc->isFeatureEnabled(bt::UT_PEX_FEATURE));
			}
		}
		else
		{
			check_data->setEnabled(false);
			peer_sources_menu->setEnabled(false);	
		}
		
		dirs_menu->setEnabled(en_dirs);
		speed_limits->setEnabled(sel.count() == 1);

		popup(pos);
	}
	
	void ViewMenu::updateGroupsMenu()
	{
		add_to_group->clear();
		for (GroupManager::iterator i = gman->begin();i != gman->end();i++)
			add_to_group->addAction(i->first);
	}

	void ViewMenu::groupsMenuItemTriggered(QAction* act)
	{
		QList<bt::TorrentInterface*> sel;
		Group* g = gman->find(act->text().remove('&')); // remove the ampersands added by Qt
	//	Out(SYS_GEN|LOG_DEBUG) << "groupsMenuItemTriggered " << act->text() << endl;
		if (!g)
			return;

		view->getSelection(sel);
		foreach (bt::TorrentInterface* tc,sel)
		{
			g->addTorrent(tc);
		}
		gman->saveGroups();
	}

}
#include "viewmenu.moc"

