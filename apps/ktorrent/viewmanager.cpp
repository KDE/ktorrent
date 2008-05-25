/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
#include <kconfig.h>
#include <klocale.h>
#include <ktabwidget.h>
#include <interfaces/torrentinterface.h>
#include <groups/group.h>
#include "viewmanager.h"
#include "ktorrentview.h"
#include "ktorrent.h"
		
typedef QValueList<KTorrentView*>::iterator ViewItr;

ViewManager::ViewManager ( QObject *parent, const char *name )
		: QObject ( parent, name ),current(0)
{}


ViewManager::~ViewManager()
{}

KTorrentView* ViewManager::newView()
{
	KTorrentView* v = new KTorrentView(0);
	views.append(v);
	return v;
}
	
void ViewManager::saveViewState(KConfig* cfg)
{
	QStringList cv;
	int idx = 0;
	for (ViewItr i = views.begin();i != views.end();i++)
	{
		cv.append((*i)->getCurrentGroup()->groupName());
		(*i)->saveSettings(cfg,idx++);	
	}
	cfg->setGroup("ViewManager");
	cfg->writeEntry("current_views",cv);
}
	
void ViewManager::restoreViewState(KConfig* cfg,KTorrent* ktor)
{
	QStringList def;
	def.append(i18n("Downloads"));
	def.append(i18n("Uploads"));
	
	cfg->setGroup("ViewManager");
	
	QStringList to_load = cfg->readListEntry("current_views",def);
	if (to_load.empty())
		to_load = def;
	
	for (QStringList::iterator i = to_load.begin();i != to_load.end();i++)
	{
		ktor->openView(*i);
	}
	
	if (views.count() == 0)
	{
		// no view open, so open default ones
		for (QStringList::iterator i = def.begin();i != def.end();i++)
			ktor->openView(*i);
	}
	
	// load status for each view
	int idx = 0;
	for (ViewItr i = views.begin();i != views.end();i++)
	{
		(*i)->loadSettings(cfg,idx++);	
	}
}

void ViewManager::updateActions()
{
	if (current)
		current->updateActions();
}

void ViewManager::addTorrent(kt::TorrentInterface* tc)
{
	for (ViewItr i = views.begin();i != views.end();i++)
	{
		(*i)->addTorrent(tc);
	}
}
	
void ViewManager::removeTorrent(kt::TorrentInterface* tc)
{
	for (ViewItr i = views.begin();i != views.end();i++)
	{
		(*i)->removeTorrent(tc);
	}
}

void ViewManager::startDownloads()
{
	if (current)
		current->startDownloads();
}

void ViewManager::stopDownloads()
{
	if (current)
		current->stopDownloads();
}

void ViewManager::startAllDownloads()
{
	if (current)
		current->startAllDownloads();
}

void ViewManager::stopAllDownloads()
{
	if (current)
		current->stopAllDownloads();
}

void ViewManager::removeDownloads()
{
	if (current)
		current->removeDownloads();
}

kt::TorrentInterface* ViewManager::getCurrentTC()
{
	return current ? current->getCurrentTC() : 0;
}

void ViewManager::update()
{
	// update the caption of each view if necessary
	for (ViewItr i = views.begin();i != views.end();i++)
	{
		if (*i != current)
		{
			KTorrentView* w = *i;
			w->updateCaption();
		}
	}
	
	if (current)
		current->update();
}

void ViewManager::queueAction()
{
	if (current)
		current->queueSlot();
}

void ViewManager::checkDataIntegrity()
{
	if (current)
		current->checkDataIntegrity();
}

void ViewManager::getSelection(QValueList<kt::TorrentInterface*> & sel)
{
	if (current)
		current->getSelection(sel);
}

void ViewManager::onCurrentTabChanged(QWidget* w)
{
	KTorrentView* old = current;
	current = 0;
	for (ViewItr i = views.begin();i != views.end() && !current;i++)
	{
		if (w == *i)
			current = *i;
	}
	
	if (!current)
		current = old;
	else
	{
		current->update();
		current->updateActions();
	}
}

bool ViewManager::closeAllowed(QWidget* )
{
	return views.count() > 1;
}

void ViewManager::tabCloseRequest(kt::GUIInterface* gui,QWidget* tab)
{
	for (ViewItr i = views.begin();i != views.end();i++)
	{
		if (tab == *i)
		{
			if (current == *i)
				current = 0;
			
			gui->removeTabPage(tab);
			delete tab;
			views.erase(i);
			break;
		}
	}
}

void ViewManager::groupRenamed(kt::Group* g,KTabWidget* mtw)
{
	for (ViewItr i = views.begin();i != views.end();i++)
	{
		KTorrentView* v = *i;
		if (v->getCurrentGroup() == g)
		{
			mtw->changeTab(v,g->groupName());
			v->setIcon(g->groupIcon());
			v->setCurrentGroup(g);
		}
	}
}

void ViewManager::groupRemoved(kt::Group* g,KTabWidget* mtw,kt::GUIInterface* gui,kt::Group* all_group)
{
	for (ViewItr i = views.begin();i != views.end();)
	{
		KTorrentView* v = *i;
		if (v->getCurrentGroup() == g)
		{
			if (views.count() > 1)
			{
				// close the tab
				gui->removeTabPage(v);
				delete v;
				i = views.erase(i);
			}
			else
			{
				mtw->changeTab(v,all_group->groupName());
				v->setIcon(all_group->groupIcon());
				v->setCurrentGroup(all_group);
				i++;
			}
		}
		else
			i++;
	}
}


#include "viewmanager.moc"
