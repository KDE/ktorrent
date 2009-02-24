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
#include <QToolButton>
#include <klocale.h>
#include <kicon.h>
#include <gui/tabbarwidget.h>
#include <groups/groupmanager.h>
#include "torrentactivity.h"
#include "viewmanager.h"
#include "gui.h"
#include "core.h"
#include "view.h"
#include "groupview.h"
#include "queuemanagerwidget.h"


namespace kt
{
	TorrentActivity::TorrentActivity(Core* core,GUI* gui,QWidget* parent) 
		: TorrentActivityInterface(i18n("Torrents"),"ktorrent",parent),core(core),gui(gui)
	{
		view_man = new ViewManager(core->getGroupManager()->allGroup(),gui,core,this);
		view_man->setupActions();
		
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		vsplit = new QSplitter(Qt::Vertical,this);
		layout->addWidget(vsplit);
		hsplit = new QSplitter(Qt::Horizontal,vsplit);
		
		tabs = new KTabWidget(hsplit);
		connect(tabs,SIGNAL(currentChanged(int)),this,SLOT(currentTabPageChanged(int)));
		group_view = new GroupView(core->getGroupManager(),view_man,gui,hsplit);
		connect(group_view,SIGNAL(openNewTab(kt::Group*)),this,SLOT(openNewView(kt::Group*)));
		
		hsplit->addWidget(group_view);
		hsplit->addWidget(tabs);
		vsplit->addWidget(hsplit);
		tool_views = new TabBarWidget(vsplit,this);
		layout->addWidget(tool_views);
		
		
		qm = new QueueManagerWidget(core->getQueueManager(),this);
		connect(core,SIGNAL(torrentAdded(bt::TorrentInterface*)),qm,SLOT(onTorrentAdded(bt::TorrentInterface*)));
		connect(core,SIGNAL(torrentRemoved(bt::TorrentInterface*)),qm,SLOT(onTorrentRemoved(bt::TorrentInterface*)));
		tool_views->addTab(qm,i18n("Queue Manager"),"kt-queue-manager",i18n("Widget to manage the torrent queue"));
		
		QToolButton* lc = new QToolButton(tabs);
		tabs->setCornerWidget(lc,Qt::TopLeftCorner);
		QToolButton* rc = new QToolButton(tabs);
		tabs->setCornerWidget(rc,Qt::TopRightCorner);
		lc->setIcon(KIcon("tab-new"));
		connect(lc,SIGNAL(clicked()),this,SLOT(newView()));
		rc->setIcon(KIcon("tab-close"));
		connect(rc,SIGNAL(clicked()),this,SLOT(closeTab()));
	}
	
	TorrentActivity::~TorrentActivity() 
	{
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
	
	void TorrentActivity::openView(const QString & group_name,bool starting_up)
	{
		Group* g = core->getGroupManager()->find(group_name);
		if (!g)
		{
			g = core->getGroupManager()->findDefault(group_name);
			if (!g)
				g = core->getGroupManager()->allGroup();
		}
		
		View* v = newView(g);
		if (!starting_up) // if it is a new view, setup the default columns based upon the group
			v->setupDefaultColumns();
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
		
		g = cfg->group("MainTabWidget");
		int ct = g.readEntry("current_tab",0);
		if (ct >= 0 && ct < tabs->count())
			tabs->setCurrentIndex(ct);
		
		view_man->loadState(cfg);
		group_view->loadState(cfg);
		qm->loadState(cfg);
		tool_views->loadState(cfg,"TorrentActivityBottomTabBar");
		notifyViewListeners(view_man->getCurrentTorrent());
	}
	
	void TorrentActivity::saveState(KSharedConfigPtr cfg)
	{
		view_man->saveState(cfg);
		group_view->saveState(cfg);
		qm->saveState(cfg);
		tool_views->saveState(cfg,"TorrentActivityBottomTabBar");
		
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
	}
	
	void TorrentActivity::updateActions()
	{
		view_man->updateActions();
	}
	
	void TorrentActivity::update()
	{
		view_man->update();
		if (qm->isVisible())
			qm->update();
	}
}
