/***************************************************************************
 *   Copyright (C) 2012 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
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

#include "groupswitcher.h"
#include <view/view.h>
#include <torrent/queuemanager.h>
#include <KConfigGroup>
#include <KIcon>
#include <QAction>
#include <QToolButton>
#include <QHBoxLayout>

namespace kt
{

	GroupSwitcher::GroupSwitcher(View* view, GroupManager* gman, QWidget* parent)
		: QWidget(parent),
		  new_tab(new QToolButton(this)),
		  close_tab(new QToolButton(this)),
		  tool_bar(new KToolBar(this)),
		  action_group(new QActionGroup(this)),
		  gman(gman),
		  view(view),
		  update_needed(true)
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->addWidget(new_tab);
		layout->addWidget(tool_bar);
		layout->addWidget(close_tab);
		//layout->setSpacing(0);
		layout->setMargin(0);
		
		new_tab->setIcon(KIcon("list-add"));
		new_tab->setToolButtonStyle(Qt::ToolButtonIconOnly);
		new_tab->setToolTip(i18n("Open a new tab"));
		connect(new_tab, SIGNAL(clicked(bool)), this, SLOT(newTab()));
		
		close_tab->setIcon(KIcon("list-remove"));
		close_tab->setToolButtonStyle(Qt::ToolButtonIconOnly);
		close_tab->setToolTip(i18n("Close the current tab"));
		connect(close_tab, SIGNAL(clicked(bool)), this, SLOT(closeTab()));
		
		tool_bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		action_group->setExclusive(true);
		connect(action_group, SIGNAL(selected(QAction*)), this, SLOT(onActivated(QAction*)));
	}

	GroupSwitcher::~GroupSwitcher()
	{
	}

	void GroupSwitcher::loadState(KSharedConfig::Ptr cfg)
	{
		KConfigGroup g = cfg->group("GroupSwitcher");

		QStringList default_groups;
		default_groups << "/all" << "/all/downloads" << "/all/uploads";

		QStringList groups = g.readEntry("groups", default_groups);
		foreach(const QString & group, groups)
		{
			addTab(gman->findByPath(group));
		}
		
		if(tabs.isEmpty())
		{
			foreach(const QString & group, default_groups)
				addTab(gman->findByPath(group));
		}
		
		int current_tab = g.readEntry("current_tab", 0);
		if(current_tab >= 0 && current_tab < tabs.count())
		{
			tabs.at(current_tab).second->setChecked(true);
			view->setGroup(tabs.at(current_tab).first);
		}
		else
		{
			tabs.first().second->setChecked(true);
			view->setGroup(tabs.first().first);
		}
	}

	void GroupSwitcher::saveState(KSharedConfig::Ptr cfg)
	{
		KConfigGroup g = cfg->group("GroupSwitcher");
		QStringList groups;
		int current_tab = 0;
		int idx = 0;
		for(QList<QPair<Group*, QAction*> >::iterator i = tabs.begin(); i != tabs.end(); i++)
		{
			groups << i->first->groupPath();
			if(i->second->isChecked())
				current_tab = idx;
			idx++;
		}
		
		g.writeEntry("groups", groups);
		g.writeEntry("current_tab", current_tab);
	}

	void GroupSwitcher::addTab(Group* group)
	{
		if(!group)
			return;

		QAction* action = tool_bar->addAction(group->groupIcon(), group->groupName());
		action->setCheckable(true);
		action_group->addAction(action);
		tabs.append(qMakePair(group, action));

		action->toggle();
		view->setGroup(group);
		
		close_tab->setEnabled(tabs.count() > 1);
	}

	void GroupSwitcher::newTab()
	{
		addTab(gman->allGroup());
	}

	void GroupSwitcher::closeTab()
	{
		if(tabs.size() <= 1) // Need atleast one tab visiblle
			return;

		QList<QPair<Group*, QAction*> >::iterator i = tabs.begin();
		while(i != tabs.end())
		{
			if(i->second->isChecked())
			{
				action_group->removeAction(i->second);
				tool_bar->removeAction(i->second);
				i->second->deleteLater();
				tabs.erase(i);
				tabs.first().second->toggle();
				view->setGroup(tabs.first().first);
				break;
			}
			i++;
		}
		
		close_tab->setEnabled(tabs.count() > 1);
	}

	void GroupSwitcher::onActivated(QAction* action)
	{
		for(QList<QPair<Group*, QAction*> >::iterator i = tabs.begin(); i != tabs.end(); i++)
		{
			if(i->second == action)
			{
				view->setGroup(i->first);
				break;
			}
		}
	}
	
	void GroupSwitcher::currentGroupChanged(Group* group)
	{
		for(QList<QPair<Group*, QAction*> >::iterator i = tabs.begin(); i != tabs.end(); i++)
		{
			if(i->second->isChecked())
			{
				i->first = group;
				i->second->setText(group->groupName());
				i->second->setIcon(group->groupIcon());
				break;
			}
		}
	}
	
	void GroupSwitcher::update(QueueManager* qman)
	{
		if(!update_needed)
			return;
		
		update_needed = false;
		for(QList<QPair<Group*, QAction*> >::iterator i = tabs.begin(); i != tabs.end(); i++)
		{
			i->first->updateCount(qman);
			i->second->setText(i->first->groupName() + QString(" %1/%2").arg(i->first->runningTorrents()).arg(i->first->totalTorrents()));
		}
	}
	
	void GroupSwitcher::queueOrdered()
	{
		update_needed = true;
	}

}
