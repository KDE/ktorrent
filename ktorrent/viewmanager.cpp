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
#include <QWidget>
#include <QClipboard>
#include <QApplication>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kmenu.h>
#include <kshortcut.h>
#include <groups/group.h>
#include <util/log.h>
#include <groups/groupmanager.h>
#include "gui.h"
#include "view.h"
#include "viewmodel.h"
#include "viewmanager.h"
#include "core.h"
#include "groupview.h"
#include "settings.h"

using namespace bt;

namespace kt
{
	ViewManager::ViewManager(Group* all_group,GUI* gui,Core* core) : QObject(gui),gui(gui),core(core),current(0),all_group(all_group)
	{
		view_menu = 0;
		model = new ViewModel(core,this);
	}

	ViewManager::~ViewManager()
	{
	}
		
	/// Create a new view
	View* ViewManager::newView(Core* core,QWidget* parent)
	{
		View* v = new View(model,core,parent);
		views.append(v);
		connect(v,SIGNAL(currentTorrentChanged(View* ,bt::TorrentInterface* )),
			this,SLOT(onCurrentTorrentChanged(View* ,bt::TorrentInterface* )));
		connect(v,SIGNAL(enableActions(View*, ActionEnableFlags)),
				this,SLOT(onEnableActions(View*, ActionEnableFlags)));
		connect(v,SIGNAL(showMenu(View*, const QPoint&)),this,SLOT(showViewMenu(View*, const QPoint&)));
		return v;
	}
		
	/// Save all views
	void ViewManager::saveState(KSharedConfigPtr cfg)
	{
		int idx = 0;
		foreach (View* v,views)
			v->saveState(cfg,idx++);

		KConfigGroup g = cfg->group("ViewManager");
		QStringList cv;
		foreach (View* v,views)
		{
			cv << v->getGroup()->groupName();
		}
		g.writeEntry("current_views",cv);
	}
		
	/// Restore all views from configuration
	void ViewManager::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("ViewManager");
		QStringList cv;
		cv = g.readEntry("current_views",cv);

		foreach (const QString &s,cv)
			gui->openView(s,true);

		if (views.count() == 0)
			gui->openNewView(all_group);

		int idx = 0;
		foreach (View* v,views)
			v->loadState(cfg,idx++);
	}
		
	void ViewManager::startTorrents()
	{
		if (current)
		{
			current->startTorrents();
			current->updateFlags();
		}
	}
		
	void ViewManager::stopTorrents()
	{
		if (current)
		{
			current->stopTorrents();
			current->updateFlags();
		}
	}
		
	void ViewManager::startAllTorrents()
	{
		if (current)
		{
			current->startAllTorrents();
			current->updateFlags();
		}
	}
		
	void ViewManager::stopAllTorrents()
	{
		if (current)
		{
			current->stopAllTorrents();
			current->updateFlags();
		}
	}
		
	void ViewManager::removeTorrents()
	{
		if (current)
		{
			current->removeTorrents();
			current->updateFlags();
		}
	}

	void ViewManager::renameTorrent()
	{
		if (current)
			current->renameTorrent();
	}

	void ViewManager::removeTorrentsAndData()
	{
		if (current)
		{
			current->removeTorrentsAndData();
			current->updateFlags();
		}
	}
		
	void ViewManager::addPeers()
	{
		if (current)
			current->addPeers();
	}

	void ViewManager::toggleDHT()
	{
		if (current)
			current->toggleDHT();
	}
		
	void ViewManager::togglePEX()
	{
		if (current)
			current->togglePEX();
	}
		
	void ViewManager::manualAnnounce()
	{
		if (current)
			current->manualAnnounce();
	}
		
	void ViewManager::scrape()
	{
		if (current)
			current->scrape();
	}
		
	void ViewManager::previewTorrents()
	{
		if (current)
			current->previewTorrents();
	}
		
	void ViewManager::openDataDir()
	{
		if (current)
			current->openDataDir();
	}
		
	void ViewManager::openTorDir()
	{
		if (current)
			current->openTorDir();
	}
		
	void ViewManager::moveData()
	{
		if (current)
			current->moveData();
	}
		
	void ViewManager::removeFromGroup()
	{
		if (current)
			current->removeFromGroup();
	}
		
	void ViewManager::checkData()
	{
		if (current)
			current->checkData();
	}
		

	void ViewManager::queueTorrents()
	{
		if (current)
			current->queueTorrents();
	}

	void ViewManager::update()
	{
		model->update();
		// check for all views if the caption needs to be updated
		// and update the current view when we come accross it
		foreach (View* v,views)
		{
			if (v == current && v->update())
				gui->changeTabText(v,v->caption());
			else if (v != current && v->needToUpdateCaption())
				gui->changeTabText(v,v->caption());
		}

	}
	
	const bt::TorrentInterface* ViewManager::getCurrentTorrent() const
	{
		return current ? current->getCurrentTorrent() : 0;
	}
	
	bt::TorrentInterface* ViewManager::getCurrentTorrent()
	{
		return current ? current->getCurrentTorrent() : 0;
	}

		
	void ViewManager::getSelection(QList<bt::TorrentInterface*> & sel)
	{
		if (current)
			current->getSelection(sel);
	}
		
	void ViewManager::onCurrentTabChanged(QWidget* tab)
	{
		foreach (View* v,views)
		{
			if (v == tab)
			{
				current = v;
				current->updateFlags();
				//Out(SYS_GEN|LOG_DEBUG) << "onCurrentTabChanged " << current->caption() << endl;
				break;
			}
		}
	}

	bool ViewManager::closeAllowed(QWidget* )
	{
		return views.count() > 1;
	}

	void ViewManager::tabCloseRequest(kt::GUIInterface* gui,QWidget* tab)
	{
		if (views.count() <= 1)
			return;

		foreach (View* v,views)
		{
			if (v == tab)
			{
				views.removeAll(v);
				gui->removeTabPage(v);				
				v->deleteLater();
				break;
			}
		}
	}
	
	void ViewManager::onCurrentGroupChanged(kt::Group* g)
	{
		if (current && current->getGroup() != g)
		{
			current->setGroup(g);
			gui->changeTabIcon(current,g->groupIconName());
			gui->changeTabText(current,current->caption());
		}
	}

	void ViewManager::onGroupRenamed(kt::Group* g)
	{
		foreach (View* v,views)
		{
			if (v->getGroup() == g)
			{
				gui->changeTabIcon(v,g->groupIconName());
				gui->changeTabText(v,v->caption());
			}
		}
		
		QMap<Group*,QAction*>::iterator j = group_actions.find(g);
		if (j != group_actions.end())
			j.value()->setText(g->groupName());
	}

	void ViewManager::onGroupRemoved(kt::Group* g)
	{
		QList<View*>::iterator i = views.begin();
		while (i != views.end())
		{
			View* v = *i;
			if (v->getGroup() == g)
			{
				if (views.count() > 1)
				{
					// remove the view 
					gui->removeTabPage(v);
					i = views.erase(i);
					v->deleteLater();
					if (current == v)
						current = 0;
				}
				else
				{
					// change the current view to the all group
					v->setGroup(all_group);
					gui->changeTabIcon(v,all_group->groupIconName());
					gui->changeTabText(v,v->caption());
					i++;
				}
			}
			else
				i++;
		}
		
		gui->unplugActionList("view_groups_list");
		QMap<Group*,QAction*>::iterator j = group_actions.find(g);
		if (j != group_actions.end())
		{
			delete j.value();
			group_actions.erase(j);
		}
		gui->plugActionList("view_groups_list",group_actions.values());
	}
	
	void ViewManager::onGroupAdded(kt::Group* g)
	{
		gui->unplugActionList("view_groups_list");
		KAction* act = new KAction(KIcon("application-x-bittorrent"),g->groupName(),this);
		connect(act,SIGNAL(triggered()),this,SLOT(addToGroupItemTriggered()));
		group_actions.insert(g,act);
		gui->plugActionList("view_groups_list",group_actions.values());
	}

	void ViewManager::onCurrentTorrentChanged(View* v,bt::TorrentInterface* tc)
	{
		if (v == current)
			gui->currentTorrentChanged(tc);
	}

	void ViewManager::onEnableActions(View* v,ActionEnableFlags flags)
	{
		if (v == current)
			enableActions(flags);
	}
	
	void ViewManager::setupActions()
	{
		KActionCollection* ac = gui->actionCollection();
		
		start_torrent = ac->action("start");
		stop_torrent = ac->action("stop");
		remove_torrent = ac->action("remove");
		
		remove_torrent_and_data = new KAction(KIcon("kt-remove"),i18n("Remove Torrent and Data"),this);
		remove_torrent_and_data->setShortcut(KShortcut(Qt::CTRL + Qt::Key_Delete));
		connect(remove_torrent_and_data,SIGNAL(triggered()),this,SLOT(removeTorrentsAndData()));
		ac->addAction("view_remove_torrent_and_data",remove_torrent_and_data);
		
		rename_torrent = new KAction(i18n("Rename Torrent"),this);
		connect(rename_torrent,SIGNAL(triggered()),this,SLOT(renameTorrent()));
		ac->addAction("view_rename_torrent",rename_torrent);
		
		queue_torrent = ac->action("queue_action");
		
		add_peers = new KAction(KIcon("list-add"),i18n("Add Peers"),this);
		connect(add_peers,SIGNAL(triggered()),this,SLOT(addPeers()));
		ac->addAction("view_add_peers",add_peers);
		
		dht_enabled = new KAction(i18n("DHT"),this);
		connect(dht_enabled,SIGNAL(triggered()),this,SLOT(toggleDHT()));
		dht_enabled->setCheckable(true);
		ac->addAction("view_dht_enabled",dht_enabled);
		
		
		pex_enabled = new KAction(i18n("Peer Exchange"),this);
		connect(pex_enabled,SIGNAL(triggered()),this,SLOT(togglePEX()));
		pex_enabled->setCheckable(true);
		ac->addAction("view_pex_enabled",pex_enabled);
	
		manual_announce = new KAction(i18n("Manual Announce"),this);
		manual_announce->setShortcut(KShortcut(Qt::CTRL + Qt::Key_A));
		connect(manual_announce,SIGNAL(triggered()),this,SLOT(manualAnnounce()));
		ac->addAction("view_announce",manual_announce);
		
		do_scrape = new KAction(i18n("Scrape"),this);
		connect(do_scrape,SIGNAL(triggered()),this,SLOT(scrape()));
		ac->addAction("view_scrape",do_scrape);
		
		preview = new KAction(KIcon("document-open"),i18n("Preview"),this);
		connect(preview,SIGNAL(triggered()),this,SLOT(previewTorrents()));
		ac->addAction("view_preview",preview);
		
		data_dir = new KAction(KIcon("folder-open"),i18n("Data Directory"),this);
		connect(data_dir,SIGNAL(triggered()),this,SLOT(openDataDir()));
		ac->addAction("view_open_data_dir",data_dir);
		
		tor_dir = new KAction(KIcon("folder-open"),i18n("Temporary Directory"),this);
		connect(tor_dir,SIGNAL(triggered()),this,SLOT(openTorDir()));
		ac->addAction("view_open_tmp_dir",tor_dir);
		
		move_data = new KAction(i18n("Move Data"),this);
		connect(move_data,SIGNAL(triggered()),this,SLOT(moveData()));
		ac->addAction("view_move_data",move_data);
		
		
		remove_from_group = new KAction(i18n("Remove from Group"),this);
		connect(remove_from_group,SIGNAL(triggered()),this,SLOT(removeFromGroup()));
		ac->addAction("view_remove_from_group",remove_from_group);
		
		add_to_new_group = new KAction(KIcon("document-new"),i18n("New Group"),this);
		connect(add_to_new_group,SIGNAL(triggered()),this,SLOT(addToNewGroup()));
		ac->addAction("view_add_to_new_group",add_to_new_group);
		
		check_data = ac->action("check_data");
		
		speed_limits =  ac->action("speed_limits");
		
		GroupManager* gman = core->getGroupManager();
		for (GroupManager::iterator i = gman->begin();i != gman->end();i++)
		{
			KAction* act = new KAction(KIcon("application-x-bittorrent"),i->first,this);
			connect(act,SIGNAL(triggered()),this,SLOT(addToGroupItemTriggered()));
			group_actions.insert(i->second,act);
		}
		
		open_dir_menu = new KAction(KIcon("folder-open"),i18n("Open Directory"),this);
		ac->addAction("OpenDirMenu",open_dir_menu);
		
		groups_menu = new KAction(KIcon("application-x-bittorrent"),i18n("Add to Group"),this);
		ac->addAction("GroupsSubMenu",groups_menu);
		
		copy_url = new KAction(KIcon("edit-copy"),i18n("Copy Torrent URL"),this);
		connect(copy_url,SIGNAL(triggered()),this,SLOT(copyTorrentURL()));
		ac->addAction("view_copy_url",copy_url);
	}
	
	void ViewManager::addToGroupItemTriggered()
	{
		if (!current)
			return;
		
		QAction* s = (QAction*)sender();
		Group* g = 0;
		QMap<Group*,QAction*>::iterator j = group_actions.begin();
		while (j != group_actions.end() && !g)
		{
			if (j.value() == s)
				g = j.key();
			j++;
		}
		
		if (!g)
			return;
		
		
		QList<bt::TorrentInterface*> sel;
		current->getSelection(sel);
		foreach (bt::TorrentInterface* tc,sel)
		{
			g->addTorrent(tc,false);
		}
		core->getGroupManager()->saveGroups();
	}
	
	void ViewManager::addToNewGroup()
	{
		GroupView* gv = gui->getGroupView();
		Group* g = gv->addNewGroup();
		if (g && current)
		{
			QList<bt::TorrentInterface*> sel;
			current->getSelection(sel);
			foreach (bt::TorrentInterface* tc,sel)
			{
				g->addTorrent(tc,false);
			}
			core->getGroupManager()->saveGroups();
		}
	}
	
	void ViewManager::copyTorrentURL()
	{
		if (!current)
			return;
		
		QList<bt::TorrentInterface*> sel;
		current->getSelection(sel);
		if (sel.count() == 0)
			return;
		
		bt::TorrentInterface* tc = sel.front();
		if (!tc->loadUrl().isValid())
			return;
		
		QClipboard* cb = QApplication::clipboard();
		cb->setText(tc->loadUrl().prettyUrl());
	}
	
	void ViewManager::showViewMenu(View* v,const QPoint & pos)
	{
		if (!v)
			return;
		
		if (!view_menu)
		{
			view_menu = (KMenu*)gui->container("ViewMenu");
			if (!view_menu)
			{
				Out(SYS_GEN|LOG_NOTICE) << "Failed to create ViewMenu" << endl;
				return;
			}
			gui->plugActionList("view_groups_list",group_actions.values());
		}
		
		bool en_start = false;
		bool en_stop = false;
		bool en_remove = false;
		bool en_prev = false;
		bool en_announce = false;
		bool en_add_peer = false;
		bool en_dirs = false;
		bool en_peer_sources = false;
		bool dummy = false;
		bool en_rename = false;

		QList<bt::TorrentInterface*> sel;
		v->getSelection(sel);
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
		do_scrape->setEnabled(sel.count() > 0);
		move_data->setEnabled(sel.count() > 0);
		queue_torrent->setEnabled(en_remove);

		const kt::Group* current_group = v->getGroup();
		remove_from_group->setEnabled(current_group && !current_group->isStandardGroup());
	
		groups_menu->setEnabled(group_actions.count() > 0);

		if (sel.count() == 1)
		{
			en_rename = true;

			//enable directories
			en_dirs = true;
			
			TorrentInterface* tc = sel.front();
			// no data check when we are preallocating diskspace
			check_data->setEnabled(tc->getStats().status != bt::ALLOCATING_DISKSPACE && !tc->isCheckingData(dummy));
			
			//enable additional peer sources if torrent is not private
			dht_enabled->setEnabled(en_peer_sources);
			pex_enabled->setEnabled(en_peer_sources);
			
			if (en_peer_sources)
			{
				dht_enabled->setChecked(tc->isFeatureEnabled(bt::DHT_FEATURE));
				pex_enabled->setChecked(tc->isFeatureEnabled(bt::UT_PEX_FEATURE));
			}
		}
		else
		{
			check_data->setEnabled(false);
			dht_enabled->setEnabled(false);	
			pex_enabled->setEnabled(false);	
		}
		
		rename_torrent->setEnabled(en_rename);
		data_dir->setEnabled(en_dirs);
		tor_dir->setEnabled(en_dirs);
		open_dir_menu->setEnabled(en_dirs);
		speed_limits->setEnabled(sel.count() == 1);
		add_to_new_group->setEnabled(sel.count() > 0);
		
		gui->unplugActionList("view_columns_list");
		gui->plugActionList("view_columns_list",v->columnActionList());

		// disable DHT and PEX if they are globally disabled
		dht_enabled->setEnabled(Settings::dhtSupport());
		pex_enabled->setEnabled(Settings::pexEnabled());
		copy_url->setEnabled(sel.count() == 1 && sel.front()->loadUrl().isValid());
		view_menu->popup(pos);
	}
}

#include "viewmanager.moc"
