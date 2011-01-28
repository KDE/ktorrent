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
#include <KSharedConfig>
#include <KConfigGroup>
#include <KAction>
#include <KActionCollection>
#include <KLocale>
#include <KMenu>
#include <KShortcut>
#include <KFileDialog>
#include <KMessageBox>
#include <groups/group.h>
#include <util/log.h>
#include <util/indexofcompare.h>
#include <groups/groupmanager.h>
#include <torrent/jobqueue.h>
#include <torrent/jobprogresswidget.h>
#include <torrent/torrentcontrol.h>
#include <interfaces/functions.h>
#include "gui.h"
#include "view.h"
#include "viewmodel.h"
#include "viewmanager.h"
#include "core.h"
#include "groups/groupview.h"
#include "settings.h"
#include "torrentactivity.h"
#include "dialogs/speedlimitsdlg.h"
#include "viewdelegate.h"
#include "scanextender.h"
#include <util/fileops.h>


using namespace bt;

namespace kt
{
	ViewManager::ViewManager(Group* all_group,GUI* gui,Core* core,TorrentActivity* ta) 
		: JobTracker(ta),gui(gui),core(core),current(0),all_group(all_group),ta(ta)
	{
	}

	ViewManager::~ViewManager()
	{
	}
		
	/// Create a new view
	View* ViewManager::newView(Core* core,QWidget* parent)
	{
		View* v = new View(core,parent);
		views.append(v);
		connect(v,SIGNAL(currentTorrentChanged(View* ,bt::TorrentInterface* )),
			this,SLOT(onCurrentTorrentChanged(View* ,bt::TorrentInterface* )));
		connect(v,SIGNAL(torrentSelectionChanged(View*)),
				this,SLOT(onSelectionChanged(View*)));
		connect(v,SIGNAL(showMenu(View*, const QPoint&)),this,SLOT(showViewMenu(View*, const QPoint&)));
		return v;
	}
	
		
	/// Save all views
	void ViewManager::saveState(KSharedConfigPtr cfg,QTabWidget* tabs)
	{
		// Sort so that they are in the same order as in the tab widget
		qSort(views.begin(),views.end(),IndexOfCompare<QTabWidget,View>(tabs));
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
			ta->openView(s,true);

		if (views.count() == 0)
			ta->openNewView(all_group);

		int idx = 0;
		foreach (View* v,views)
			v->loadState(cfg,idx++);
	}
		
	void ViewManager::startTorrents()
	{
		if (current)
			current->startTorrents();
	}
		
	void ViewManager::stopTorrents()
	{
		if (current)
			current->stopTorrents();
	}
		
	void ViewManager::startAllTorrents()
	{
		if (current)
			current->startAllTorrents();
	}
		
	void ViewManager::stopAllTorrents()
	{
		if (current)
			current->stopAllTorrents();
	}
		
	void ViewManager::pauseTorrents()
	{
		if (current)
			current->pauseTorrents();
	}

	void ViewManager::removeTorrents()
	{
		if (current)
			current->removeTorrents();
	}

	void ViewManager::renameTorrent()
	{
		if (current)
			current->renameTorrent();
	}

	void ViewManager::removeTorrentsAndData()
	{
		if (current)
			current->removeTorrentsAndData();
	}
		
	void ViewManager::addPeers()
	{
		if (current)
			current->addPeers();
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

	void ViewManager::showProperties()
	{
		if (current)
			current->showProperties();
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

	void ViewManager::update()
	{
		foreach (View* v,views)
		{
			if (current == v)
				current->update();
			else if (v->needToUpdateCaption())
				ta->setTabProperties(v,v->caption(false),v->getGroup()->groupIconName(),v->caption(true));
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
		current = 0;
		foreach (View* v,views)
		{
			if (v == tab)
			{
				current = v;
				updateActions();
				//Out(SYS_GEN|LOG_DEBUG) << "onCurrentTabChanged " << current->caption() << endl;
				current->update();
				break;
			}
		}
	}

	void ViewManager::removeView(View* view)
	{
		if (views.count() <= 1)
			return;

		foreach (View* v,views)
		{
			if (v == view)
			{
				views.removeAll(v);
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
			ta->setTabProperties(current,current->caption(false),g->groupIconName(),current->caption(true));
		}
	}

	void ViewManager::onGroupRenamed(kt::Group* g)
	{
		foreach (View* v,views)
		{
			if (v->getGroup() == g)
			{
				ta->setTabProperties(v,v->caption(false),g->groupIconName(),v->caption(true));
			}
		}
		
		QMap<Group*,KAction*>::iterator j = group_actions.find(g);
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
					ta->removeView(v);
					i = views.erase(i);
					v->deleteLater();
					if (current == v)
						current = 0;
				}
				else
				{
					// change the current view to the all group
					v->setGroup(all_group);
					ta->setTabProperties(v,v->caption(false),g->groupIconName(),v->caption(true));
					i++;
				}
			}
			else
				i++;
		}
		
		gui->getTorrentActivity()->part()->unplugActionList("view_groups_list");
		QList<QAction*> actions;
		QMap<Group*,KAction*>::iterator j = group_actions.begin();
		while (j != group_actions.end())
		{
			if (j.key() == g)
			{
				delete j.value();
				j = group_actions.erase(j);
			}
			else
			{
				actions.append(j.value());
				j++;
			}
		}
		
		gui->getTorrentActivity()->part()->plugActionList("view_groups_list",actions);
	}
	
	void ViewManager::onGroupAdded(kt::Group* g)
	{
		gui->getTorrentActivity()->part()->unplugActionList("view_groups_list");
		KAction* act = new KAction(KIcon("application-x-bittorrent"),g->groupName(),this);
		connect(act,SIGNAL(triggered()),this,SLOT(addToGroupItemTriggered()));
		group_actions.insert(g,act);
		
		QList<QAction*> actions;
		QMap<Group*,KAction*>::iterator j = group_actions.begin();
		while (j != group_actions.end())
		{
			actions.append(j.value());
			j++;
		}
		
		gui->getTorrentActivity()->part()->plugActionList("view_groups_list",actions);
	}

	void ViewManager::onCurrentTorrentChanged(View* v,bt::TorrentInterface* tc)
	{
		if (v == current)
			ta->currentTorrentChanged(tc);
	}

	void ViewManager::onSelectionChanged(View* v)
	{
		if (v != current)
			return;
			
		updateActions();
	}
	
	void ViewManager::jobRegistered(bt::Job* j)
	{
		foreach (View* v,views)
		{
			JobProgressWidget* w = createJobWidget(j);
			v->extend(j->torrent(),w);
		}
	}
	
	void ViewManager::jobUnregistered(bt::Job* j)
	{
		JobProgessWidgetList & jpw = widgets[j];
		foreach (JobProgressWidget* w,jpw)
			if (w->automaticRemove())
				w->emitCloseRequest();
	}

	JobProgressWidget* ViewManager::createJobWidget(Job* job)
	{
		if (job->torrentStatus() == bt::CHECKING_DATA)
		{
			ScanExtender* ext = new ScanExtender(job,0);
			widgets[job].append(ext);
			return ext;
		}
		else
			return kt::JobTracker::createJobWidget(job);
	}

	struct StartAndStopAllVisitor
	{
		QAction* start_all;
		QAction* stop_all;
		
		StartAndStopAllVisitor(QAction* start_all,QAction* stop_all) : start_all(start_all),stop_all(stop_all)
		{}
		
		bool operator()(bt::TorrentInterface* tc)
		{
			if (tc->getJobQueue()->runningJobs())
				return true;
			
			const TorrentStats & s = tc->getStats();
			if (s.running || (tc->isAllowedToStart() && !tc->overMaxRatio() && !tc->overMaxSeedTime()))
				stop_all->setEnabled(true);
			else
				start_all->setEnabled(true);
			
			return !stop_all->isEnabled() || !start_all->isEnabled();
		}
	};
	
	void ViewManager::updateActions()
	{
		if (!current)
			return;
		
		QList<bt::TorrentInterface*> sel;
		current->getSelection(sel);		
		
		bool qm_enabled = !Settings::manuallyControlTorrents();
		bool en_start = false;
		bool en_stop = false;
		bool en_remove = false;
		bool en_prev = false;
		bool en_announce = false;
		bool en_add_peer = false;
		bool en_peer_sources = false;
		bool en_pause = false;
		bool en_completed = false;

		foreach (bt::TorrentInterface* tc,sel)
		{
			const TorrentStats & s = tc->getStats();
			
			if (tc->readyForPreview() && !s.multi_file_torrent)
				en_prev = true;
			
			if (tc->getJobQueue()->runningJobs())
				continue;
			
			en_remove = true;
			if (!s.running)
			{	
				if (qm_enabled)
				{
					// Queued torrents can be stopped, and not started
					if (s.queued)
						en_stop = true;
					else
						en_start = true;
				}
				else
				{
					en_start = true;
				}
			}
			else
			{
				en_stop = true;
				if (tc->announceAllowed())
					en_announce = true;
				
				if (!s.paused)
					en_pause = true;
				else
					en_start = true;
			}
			
			if (!s.priv_torrent)
			{
				en_add_peer = true;
				en_peer_sources = true;
			}

			if (s.completed)
				en_completed = true;
		}

		en_add_peer = en_add_peer && en_stop;

		start_torrent->setEnabled(en_start);
		stop_torrent->setEnabled(en_stop);
		remove_torrent->setEnabled(en_remove);
		remove_torrent_and_data->setEnabled(en_remove);
		pause_torrent->setEnabled(en_pause);
		preview->setEnabled(en_prev);
		add_peers->setEnabled(en_add_peer);
		manual_announce->setEnabled(en_announce);
		do_scrape->setEnabled(sel.count() > 0);
		move_data->setEnabled(sel.count() > 0);

		const kt::Group* current_group = current->getGroup();
		remove_from_group->setEnabled(current_group && !current_group->isStandardGroup());
		groups_menu->setEnabled(group_actions.count() > 0);

		if (sel.count() == 1)
		{
			TorrentInterface* tc = sel.front();
			// no data check when we are preallocating diskspace
			check_data->setEnabled(tc->getStats().status != bt::ALLOCATING_DISKSPACE);
		}
		else
		{
			check_data->setEnabled(false);
		}
		
		rename_torrent->setEnabled(sel.count() == 1);
		data_dir->setEnabled(sel.count() == 1);
		tor_dir->setEnabled(sel.count() == 1);
		open_dir_menu->setEnabled(sel.count() == 1);
		add_to_new_group->setEnabled(sel.count() > 0);
		copy_url->setEnabled(sel.count() == 1 && sel.front()->loadUrl().isValid());
		export_torrent->setEnabled(sel.count() == 1);
		
		if (qm_enabled)
		{
			start_all->setEnabled(false);
			stop_all->setEnabled(false);
			StartAndStopAllVisitor v(start_all,stop_all);
			current->viewModel()->visit(v);
		}
		else
		{
			start_all->setEnabled(current->numRunningTorrents() < current->numTorrents());
			stop_all->setEnabled(current->numRunningTorrents() > 0);
		}
		
		// check for all views if the caption needs to be updated
		foreach (View* v,views)
		{
			if (v->needToUpdateCaption())
			{
				ta->setTabProperties(v,v->caption(false),v->getGroup()->groupIconName(),v->caption(true));
			}
		}
	}
	
	void ViewManager::setupActions(KActionCollection* ac)
	{
		KStandardAction::selectAll(this,SLOT(selectAll()),ac);
		
		start_torrent = new KAction(KIcon("kt-start"),i18nc("@action Start all selected torrents in the current tab", "Start"), this);
		start_torrent->setToolTip(i18n("Start all selected torrents in the current tab"));
		start_torrent->setShortcut(KShortcut(Qt::CTRL + Qt::Key_S));
		connect(start_torrent,SIGNAL(triggered()),this,SLOT(startTorrents()));
		ac->addAction("start",start_torrent);
		
		stop_torrent = new KAction(KIcon("kt-stop"),i18nc("@action Stop all selected torrents in the current tab", "Stop"),this);
		stop_torrent->setToolTip(i18n("Stop all selected torrents in the current tab"));
		stop_torrent->setShortcut(KShortcut(Qt::CTRL + Qt::Key_H));
		connect(stop_torrent,SIGNAL(triggered()),this,SLOT(stopTorrents()));
		ac->addAction("stop",stop_torrent);
		
		pause_torrent = new KAction(KIcon("media-playback-pause"),i18nc("@action Pause all selected torrents in the current tab", "Pause"),this);
		pause_torrent->setToolTip(i18n("Pause all selected torrents in the current tab"));
		connect(pause_torrent,SIGNAL(triggered()),this,SLOT(pauseTorrents()));
		ac->addAction("pause",pause_torrent);
		
		remove_torrent = new KAction(KIcon("kt-remove"),i18nc("@action Remove all selected torrents in the current tab", "Remove"),this);
		remove_torrent->setToolTip(i18n("Remove all selected torrents in the current tab"));
		remove_torrent->setShortcut(KShortcut(Qt::Key_Delete));
		connect(remove_torrent,SIGNAL(triggered()),this,SLOT(removeTorrents()));
		ac->addAction("remove",remove_torrent);
		
		start_all = new KAction(KIcon("kt-start-all"),i18nc("@action Start all torrents in the current tab", "Start All"),this);
		start_all->setToolTip(i18n("Start all torrents in the current tab"));
		start_all->setShortcut(KShortcut(Qt::SHIFT + Qt::Key_S));
		connect(start_all,SIGNAL(triggered()),this,SLOT(startAllTorrents()));
		ac->addAction("start_all",start_all);
		
		stop_all = new KAction(KIcon("kt-stop-all"),i18nc("@action Stop all torrents in the current tab", "Stop All"),this);
		stop_all->setToolTip(i18n("Stop all torrents in the current tab"));
		stop_all->setShortcut(KShortcut(Qt::SHIFT + Qt::Key_H));
		connect(stop_all,SIGNAL(triggered()),this,SLOT(stopAllTorrents()));
		ac->addAction("stop_all",stop_all);
		
		remove_torrent_and_data = new KAction(KIcon("kt-remove"),i18n("Remove Torrent and Data"),this);
		remove_torrent_and_data->setShortcut(KShortcut(Qt::CTRL + Qt::Key_Delete));
		connect(remove_torrent_and_data,SIGNAL(triggered()),this,SLOT(removeTorrentsAndData()));
		ac->addAction("view_remove_torrent_and_data",remove_torrent_and_data);
		
		rename_torrent = new KAction(i18n("Rename Torrent"),this);
		connect(rename_torrent,SIGNAL(triggered()),this,SLOT(renameTorrent()));
		ac->addAction("view_rename_torrent",rename_torrent);
		
		add_peers = new KAction(KIcon("list-add"),i18n("Add Peers"),this);
		connect(add_peers,SIGNAL(triggered()),this,SLOT(addPeers()));
		ac->addAction("view_add_peers",add_peers);
	
		manual_announce = new KAction(i18n("Manual Announce"),this);
		manual_announce->setShortcut(KShortcut(Qt::SHIFT + Qt::Key_A));
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

		torrent_properties = new KAction(i18n("Settings"),this);
		connect(torrent_properties,SIGNAL(triggered()),this,SLOT(showProperties()));
		ac->addAction("view_torrent_properties", torrent_properties);

		remove_from_group = new KAction(i18n("Remove from Group"),this);
		connect(remove_from_group,SIGNAL(triggered()),this,SLOT(removeFromGroup()));
		ac->addAction("view_remove_from_group",remove_from_group);
		
		add_to_new_group = new KAction(KIcon("document-new"),i18n("New Group"),this);
		connect(add_to_new_group,SIGNAL(triggered()),this,SLOT(addToNewGroup()));
		ac->addAction("view_add_to_new_group",add_to_new_group);
		
		check_data = new KAction(KIcon("kt-check-data"),i18n("Check Data"),this);
		check_data->setToolTip(i18n("Check all the data of a torrent"));
		check_data->setShortcut(KShortcut(Qt::SHIFT + Qt::Key_C));
		connect(check_data,SIGNAL(triggered()),this,SLOT(checkData()));
		ac->addAction("check_data",check_data);
		
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
		
		export_torrent = new KAction(KIcon("document-export"),i18n("Export Torrent"),this);
		connect(export_torrent,SIGNAL(triggered()),this,SLOT(exportTorrent()));
		ac->addAction("view_export_torrent",export_torrent);
		
		speed_limits = new KAction(KIcon("kt-speed-limits"),i18n("Speed Limits"),this);
		speed_limits->setToolTip(i18n("Set the speed limits of individual torrents"));
		connect(speed_limits,SIGNAL(triggered()),this,SLOT(speedLimits()));
		speed_limits->setShortcut(KShortcut(Qt::CTRL + Qt::Key_L));
		ac->addAction("speed_limits",speed_limits);
	}
	
	void ViewManager::addToGroupItemTriggered()
	{
		if (!current)
			return;
		
		KAction* s = (KAction*)sender();
		Group* g = 0;
		QMap<Group*,KAction*>::iterator j = group_actions.begin();
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
		GroupView* gv = ta->getGroupView();
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
	
	void ViewManager::selectAll()
	{
		if (current)
			current->selectAll();
	}
	
	void ViewManager::speedLimits()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		SpeedLimitsDlg dlg(sel.count() > 0 ? sel.front() : 0,core,gui->getMainWindow());
		dlg.exec();
	}
	
	void ViewManager::exportTorrent()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		if (sel.count() == 1)
		{
			bt::TorrentInterface* tc = sel.front();
			QString filter = kt::TorrentFileFilter(false);
			QString fn = KFileDialog::getSaveFileName(KUrl("kfiledialog:///exportTorrent"),filter,gui,
													  QString(),KFileDialog::ConfirmOverwrite);
			if (fn.isEmpty())
				return;
			
			KIO::file_copy(tc->getTorDir() + "torrent",fn,-1,KIO::Overwrite);
		}
	}
	
	void ViewManager::showViewMenu(View* v,const QPoint & pos)
	{
		if (!v)
			return;
		
		KMenu* view_menu = gui->getTorrentActivity()->part()->menu("ViewMenu");
		if (!view_menu)
			return;
		
		QList<QAction*> actions;
		QMap<Group*,KAction*>::iterator j = group_actions.begin();
		while (j != group_actions.end())
		{
			actions.append(j.value());
			j++;
		}
			
		gui->getTorrentActivity()->part()->plugActionList("view_groups_list",actions);
		gui->getTorrentActivity()->part()->unplugActionList("view_columns_list");
		gui->getTorrentActivity()->part()->plugActionList("view_columns_list",v->columnActionList());
		view_menu->popup(pos);
	}
}

#include "viewmanager.moc"
