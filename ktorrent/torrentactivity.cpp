/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <QBoxLayout>
#include <QToolBar>
#include <KConfigGroup>
#include <KLocale>
#include <KIcon>
#include <KToggleAction>
#include <KActionCollection>
#include <util/log.h>
#include <gui/tabbarwidget.h>
#include <groups/groupmanager.h>
#include "torrentactivity.h"
#include "gui.h"
#include "core.h"
#include "view/view.h"
#include "view/torrentsearchbar.h"
#include "groups/groupview.h"
#include "tools/queuemanagerwidget.h"
#include "tools/magnetview.h"
#include "torrent/queuemanager.h"

using namespace bt;


namespace kt
{

	
	TorrentActivity::TorrentActivity(Core* core,GUI* gui,QWidget* parent) 
		: TorrentActivityInterface(i18n("Torrents"),"ktorrent",parent),core(core),gui(gui)
	{
		setXMLGUIFile("kttorrentactivityui.rc");
		QWidget* view_part = new QWidget(this);
		view = new View(core, gui, view_part);
		connect(view, SIGNAL(currentTorrentChanged(bt::TorrentInterface*)),
				this, SLOT(currentTorrentChanged(bt::TorrentInterface*)));
		search_bar = new TorrentSearchBar(view, view_part);
		search_bar->setHidden(true);
		setupActions();
		
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		vsplit = new QSplitter(Qt::Vertical,this);
		layout->addWidget(vsplit);
		hsplit = new QSplitter(Qt::Horizontal,vsplit);
		
		QVBoxLayout* vlayout = new QVBoxLayout(view_part);
		vlayout->setSpacing(0);
		vlayout->setMargin(0);
		vlayout->addWidget(search_bar);
		vlayout->addWidget(view);
		
		group_view = new GroupView(core->getGroupManager(),view,gui,hsplit);
		group_view->setupActions(part()->actionCollection());
		
		hsplit->addWidget(group_view);
		hsplit->addWidget(view_part);
		hsplit->setStretchFactor(0,1);
		hsplit->setStretchFactor(1,3);
		vsplit->addWidget(hsplit);
		tool_views = new TabBarWidget(vsplit,this);
		vsplit->setStretchFactor(0,3);
		vsplit->setStretchFactor(1,1);
		layout->addWidget(tool_views);
		
		
		qm = new QueueManagerWidget(core->getQueueManager(),this);
		connect(core,SIGNAL(torrentAdded(bt::TorrentInterface*)),qm,SLOT(onTorrentAdded(bt::TorrentInterface*)));
		connect(core,SIGNAL(torrentRemoved(bt::TorrentInterface*)),qm,SLOT(onTorrentRemoved(bt::TorrentInterface*)));
		tool_views->addTab(qm,i18n("Queue Manager"),"kt-queue-manager",i18n("Widget to manage the torrent queue"));
		
		magnet_view = new MagnetView(core->getMagnetModel(),this);
		tool_views->addTab(magnet_view,i18n("Magnet"),"kt-magnet",
						   i18n("Displays the currently downloading magnet links"));
		
		QueueManager* qman = core->getQueueManager();
		connect(qman,SIGNAL(suspendStateChanged(bool)),this,SLOT(onSuspendedStateChanged(bool)));
		
		queue_suspend_action->setChecked(core->getSuspendedState());
	}
	
	TorrentActivity::~TorrentActivity() 
	{
	}
	
	void TorrentActivity::setupActions()
	{
		KActionCollection* ac = part()->actionCollection();
		start_all_action = new KAction(KIcon("kt-start-all"),i18n("Start All"),this);
		start_all_action->setToolTip(i18n("Start all torrents"));
		connect(start_all_action,SIGNAL(triggered()),this,SLOT(startAllTorrents()));
		ac->addAction("start_all",start_all_action);
		
		stop_all_action = new KAction(KIcon("kt-stop-all"),i18n("Stop All"),this);
		stop_all_action->setToolTip(i18n("Stop all torrents"));
		connect(stop_all_action,SIGNAL(triggered()),this,SLOT(stopAllTorrents()));
		ac->addAction("stop_all",stop_all_action);
		
		queue_suspend_action = new KToggleAction(KIcon("kt-pause"),i18n("Suspend Torrents"),this);
		ac->addAction("queue_suspend",queue_suspend_action);
		queue_suspend_action->setToolTip(i18n("Suspend all running torrents"));
		queue_suspend_action->setShortcut(KShortcut(Qt::SHIFT + Qt::Key_P));
		queue_suspend_action->setGlobalShortcut(KShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_P));
		connect(queue_suspend_action,SIGNAL(toggled(bool)),this,SLOT(suspendQueue(bool)));
		
		show_group_view_action = new KToggleAction(KIcon("view-list-tree"),i18n("Group View Visible"),this);
		show_group_view_action->setToolTip(i18n("Show or hide the group view"));
		connect(show_group_view_action,SIGNAL(toggled(bool)),this,SLOT(setGroupViewVisible(bool)));
		ac->addAction("show_group_view",show_group_view_action);
		
		filter_torrent_action = new KAction(i18n("Filter Torrents"), this);
		filter_torrent_action->setToolTip(i18n("Filter torrents based on filter string"));
		filter_torrent_action->setShortcut(Qt::CTRL + Qt::Key_F);
		connect(filter_torrent_action, SIGNAL(triggered(bool)), search_bar, SLOT(showBar()));
		ac->addAction("filter_torrent", filter_torrent_action);
		
		view->setupActions(ac);
	}

	void TorrentActivity::addToolWidget(QWidget* widget,const QString & text,const QString & icon,const QString & tooltip)
	{
		tool_views->addTab(widget,text,icon,tooltip);
	}
	
	void TorrentActivity::removeToolWidget(QWidget* widget)
	{
		tool_views->removeTab(widget);
	}
	
	void TorrentActivity::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("TorrentActivitySplitters");
		if (vsplit)
		{
			QByteArray data;
			data = QByteArray::fromBase64(g.readEntry("vsplit",data));
			vsplit->restoreState(data);
		}
		
		if (hsplit)
		{
			QByteArray data;
			data = QByteArray::fromBase64(g.readEntry("hsplit",data));
			hsplit->restoreState(data);
		}
		
		search_bar->loadState(cfg);
		view->loadState(cfg);
		group_view->loadState(cfg);
		qm->loadState(cfg);
		tool_views->loadState(cfg,"TorrentActivityBottomTabBar");
		notifyViewListeners(view->getCurrentTorrent());
		magnet_view->loadState(cfg);
		
		show_group_view_action->setChecked(!group_view->isHidden());
	}
	
	void TorrentActivity::saveState(KSharedConfigPtr cfg)
	{
		search_bar->saveState(cfg);
		view->saveState(cfg);
		group_view->saveState(cfg);
		qm->saveState(cfg);
		tool_views->saveState(cfg,"TorrentActivityBottomTabBar");
		magnet_view->saveState(cfg);
		
		KConfigGroup g = cfg->group("TorrentActivitySplitters");
		if (vsplit)
		{
			QByteArray data = vsplit->saveState();
			g.writeEntry("vsplit",data.toBase64());
		}
		
		if (hsplit)
		{
			QByteArray data = hsplit->saveState();
			g.writeEntry("hsplit",data.toBase64());
		}
	}
	
	const TorrentInterface* TorrentActivity::getCurrentTorrent() const
	{
		return view->getCurrentTorrent();
	}
	
	bt::TorrentInterface* TorrentActivity::getCurrentTorrent()
	{
		return view->getCurrentTorrent();
	}
	
	void TorrentActivity::currentTorrentChanged(bt::TorrentInterface* tc)
	{
		notifyViewListeners(tc);
	}
	
	void TorrentActivity::updateActions()
	{
		view->updateActions();
		bt::Uint32 nr = core->getNumTorrentsRunning();
		queue_suspend_action->setEnabled(core->getSuspendedState() || nr > 0);
		start_all_action->setEnabled(core->getNumTorrentsNotRunning() > 0);
		stop_all_action->setEnabled(nr > 0);
	}
	
	void TorrentActivity::update()
	{
		view->update();
		if (qm->isVisible())
			qm->update();
	}
	
	void TorrentActivity::setGroupViewVisible(bool visible)
	{
		group_view->setVisible(visible);
	}
	
	void TorrentActivity::startAllTorrents()
	{
		core->startAll();
	}
	
	void TorrentActivity::stopAllTorrents()
	{
		core->stopAll();
	}
	
	void TorrentActivity::suspendQueue(bool suspend)
	{
		Out(SYS_GEN|LOG_NOTICE) << "Setting suspended state to " << suspend << endl;
		core->setSuspendedState(suspend);
		updateActions();
	}
	
	void TorrentActivity::onSuspendedStateChanged(bool suspended)
	{
		queue_suspend_action->setChecked(suspended);
	}
	
	Group* TorrentActivity::addNewGroup()
	{
		return group_view->addNewGroup();
	}


}
