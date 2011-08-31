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
#include <QTreeView>
#include <QToolButton>
#include <KConfigGroup>
#include <KLocale>
#include <KIcon>
#include <KToggleAction>
#include <KActionCollection>
#include <util/log.h>
#include <gui/tabbarwidget.h>
#include <groups/groupmanager.h>
#include "torrentactivity.h"
#include "view/viewmanager.h"
#include "gui.h"
#include "core.h"
#include "view/view.h"
#include "groups/groupview.h"
#include "tools/queuemanagerwidget.h"
#include "tools/magnetview.h"

using namespace bt;


namespace kt
{
	
	TorrentActivity::TorrentActivity(Core* core,GUI* gui,QWidget* parent) 
		: TorrentActivityInterface(i18n("Torrents"),"ktorrent",parent),core(core),gui(gui)
	{
		setXMLGUIFile("kttorrentactivityui.rc");
		view_man = new ViewManager(core->getGroupManager()->allGroup(),gui,core,this);
		setupActions();
		
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		vsplit = new QSplitter(Qt::Vertical,this);
		layout->addWidget(vsplit);
		hsplit = new QSplitter(Qt::Horizontal,vsplit);
		
		tabs = new KTabWidget(hsplit);
		tabs->setMovable(true);
		connect(tabs,SIGNAL(currentChanged(int)),this,SLOT(currentTabPageChanged(int)));
		group_view = new GroupView(core->getGroupManager(),view_man,gui,hsplit);
		group_view->setupActions(part()->actionCollection());
		connect(group_view,SIGNAL(openNewTab(kt::Group*)),this,SLOT(openNewView(kt::Group*)));
		
		hsplit->addWidget(group_view);
		hsplit->addWidget(tabs);
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
		
		QToolButton* lc = new QToolButton(tabs);
		tabs->setCornerWidget(lc,Qt::TopLeftCorner);
		QToolButton* rc = new QToolButton(tabs);
		tabs->setCornerWidget(rc,Qt::TopRightCorner);
		lc->setIcon(KIcon("tab-new"));
		connect(lc,SIGNAL(clicked()),this,SLOT(newView()));
		rc->setIcon(KIcon("tab-close"));
		connect(rc,SIGNAL(clicked()),this,SLOT(closeTab()));
		
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
		
		view_man->setupActions(ac);
	}


	void TorrentActivity::openNewView(kt::Group* g)
	{
		View* v = newView(g);
		v->setupDefaultColumns();
	}
	
	View* TorrentActivity::newView(kt::Group* g)
	{
		View* view = view_man->newView(core,this);
		view->setGroup(g);
		int idx = tabs->addTab(view,KIcon(g->groupIconName()),view->caption(false));
		tabs->setTabToolTip(idx,view->caption(true));
		connect(view,SIGNAL(editingItem(bool)),gui,SLOT(setPasteDisabled(bool)));
		return view;
	}
	
	void TorrentActivity::newView()
	{
		newView(core->getGroupManager()->allGroup());
	}
	
	void TorrentActivity::openView(const QString & group_path)
	{
		Group* g = core->getGroupManager()->findByPath(group_path);
		if (!g)
			g = core->getGroupManager()->allGroup();
		
		newView(g);
	}
	
	void TorrentActivity::closeTab()
	{
		QWidget* w = tabs->currentWidget();
		if (!w || tabs->count() == 1)
			return;
		
		tabs->removeTab(tabs->currentIndex());
		view_man->removeView((View*)w);
	}
	
	void TorrentActivity::removeView(View* v)
	{
		tabs->removeTab(tabs->indexOf(v));
	}
	
	void TorrentActivity::setTabProperties(View* v,const QString & name,const QString & icon,const QString & tooltip)
	{
		int idx = tabs->indexOf(v);
		if (idx < 0)
			return;
		
		tabs->setTabIcon(idx,KIcon(icon));
		tabs->setTabToolTip(idx,tooltip);
		tabs->setTabText(idx,name);
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
		
		view_man->loadState(cfg);
		
		g = cfg->group("MainTabWidget");
		int ct = g.readEntry("current_tab",0);
		if (ct >= 0 && ct < tabs->count())
			tabs->setCurrentIndex(ct);
		
		group_view->loadState(cfg);
		qm->loadState(cfg);
		tool_views->loadState(cfg,"TorrentActivityBottomTabBar");
		notifyViewListeners(view_man->getCurrentTorrent());
		tabs->cornerWidget(Qt::TopRightCorner)->setEnabled(tabs->count() > 1);
		magnet_view->loadState(cfg);
		
		show_group_view_action->setChecked(!group_view->isHidden());
	}
	
	void TorrentActivity::saveState(KSharedConfigPtr cfg)
	{
		view_man->saveState(cfg,tabs);
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
		
		// save the current tab
		g = cfg->group("MainTabWidget");
		g.writeEntry("current_tab",tabs->currentIndex());
	}
	
	const TorrentInterface* TorrentActivity::getCurrentTorrent() const
	{
		return view_man->getCurrentTorrent();
	}
	
	bt::TorrentInterface* TorrentActivity::getCurrentTorrent()
	{
		return view_man->getCurrentTorrent();
	}
	
	void TorrentActivity::currentTorrentChanged(bt::TorrentInterface* tc)
	{
		notifyViewListeners(tc);
	}
	
	void TorrentActivity::currentTabPageChanged(int idx)
	{
		QWidget* page = tabs->widget(idx);
		view_man->onCurrentTabChanged(page);
		notifyViewListeners(view_man->getCurrentTorrent());
		tabs->cornerWidget(Qt::TopRightCorner)->setEnabled(tabs->count() > 1);
	}
	
	void TorrentActivity::updateActions()
	{
		view_man->updateActions();
		bt::Uint32 nr = core->getNumTorrentsRunning();
		queue_suspend_action->setEnabled(core->getSuspendedState() || nr > 0);
		start_all_action->setEnabled(core->getNumTorrentsNotRunning() > 0);
		stop_all_action->setEnabled(nr > 0);
	}
	
	void TorrentActivity::update()
	{
		view_man->update();
		if (qm->isVisible())
			qm->update();
	}
	
	
	View* TorrentActivity::getCurrentView()
	{
		return view_man->getCurrentView();
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
	

}
