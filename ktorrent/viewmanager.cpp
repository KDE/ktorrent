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
#include <ksharedconfig.h>
#include <groups/group.h>
#include "gui.h"
#include "view.h"
#include "viewmanager.h"

namespace kt
{
	ViewManager::ViewManager(Group* all_group,GUI* gui) : QObject(gui),gui(gui),current(0),all_group(all_group)
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
		connect(v,SIGNAL(currentTorrentChanged(View* ,kt::TorrentInterface* )),
			this,SLOT(onCurrentTorrentChanged(View* ,kt::TorrentInterface* )));
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

		foreach (QString s,cv)
			gui->openView(s);

		if (views.count() == 0)
			gui->openView(all_group);

		int idx = 0;
		foreach (View* v,views)
			v->loadState(cfg,idx++);
	}
		
	/// Start all selected downloads in the current view
	void ViewManager::startTorrents()
	{
		if (current)
			current->startTorrents();
	}
		
	/// Stop all selected downloads in the current view
	void ViewManager::stopTorrents()
	{
		if (current)
			current->stopTorrents();
	}
		
	/// Start all downloads in the current view
	void ViewManager::startAllTorrents()
	{
		if (current)
			current->startAllTorrents();
	}
		
	/// Stop all downloads in the current view
	void ViewManager::stopAllTorrents()
	{
		if (current)
			current->stopAllTorrents();
	}
		
	/// Remove selected downloads in the current view
	void ViewManager::removeTorrents()
	{
		if (current)
			current->removeTorrents();
	}

	void ViewManager::queueTorrents()
	{
		if (current)
			current->queueTorrents();
	}

	void ViewManager::checkData()
	{
		if (current)
			current->checkData();
	}

	/// Update the current view
	void ViewManager::update()
	{
		if (current)
			current->update();
	}
	
	const kt::TorrentInterface* ViewManager::getCurrentTorrent() const
	{
		return current ? current->getCurrentTorrent() : 0;
	}
	
	kt::TorrentInterface* ViewManager::getCurrentTorrent()
	{
		return current ? current->getCurrentTorrent() : 0;
	}

		
	void ViewManager::getSelection(QList<kt::TorrentInterface*> & sel)
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
			gui->changeTabText(current,g->groupName());
		}
	}

	void ViewManager::onGroupRenamed(kt::Group* g)
	{
		foreach (View* v,views)
		{
			if (v->getGroup() == g)
			{
				gui->changeTabIcon(v,g->groupIconName());
				gui->changeTabText(v,g->groupName());
			}
		}
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
					gui->changeTabText(v,all_group->groupName());
					i++;
				}
			}
			else
				i++;
		}
	}

	void ViewManager::onCurrentTorrentChanged(View* v,kt::TorrentInterface* tc)
	{
		if (v == current)
			gui->currentTorrentChanged(tc);
	}

}

#include "viewmanager.moc"
