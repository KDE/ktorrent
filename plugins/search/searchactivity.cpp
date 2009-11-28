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
#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>
#include <QToolButton>
#include <klocale.h>
#include <kicon.h>
#include "searchactivity.h"
#include "searchwidget.h"
#include "searchplugin.h"
#include <interfaces/functions.h>


namespace kt
{
	SearchActivity::SearchActivity(SearchPlugin* sp,QWidget* parent)
		: Activity(i18nc("plugin name","Search"),"edit-find",10,parent),sp(sp)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		tabs = new KTabWidget(this);
		layout->addWidget(tabs);
		connect(tabs,SIGNAL(currentChanged(int)),this,SLOT(currentTabChanged(int)));
		
		QToolButton* lc = new QToolButton(tabs);
		tabs->setCornerWidget(lc,Qt::TopLeftCorner);
		QToolButton* rc = new QToolButton(tabs);
		tabs->setCornerWidget(rc,Qt::TopRightCorner);
		lc->setIcon(KIcon("tab-new"));
		connect(lc,SIGNAL(clicked()),this,SLOT(openTab()));
		rc->setIcon(KIcon("tab-close"));
		connect(rc,SIGNAL(clicked()),this,SLOT(closeTab()));
	}
	
	SearchActivity::~SearchActivity()
	{
	}
	
	void SearchActivity::search(const QString & text,int engine)
	{
		foreach (SearchWidget* s,searches)
		{
			if (s->getCurrentUrl() == KUrl("about:ktorrent"))
			{
				s->search(text,engine);
				tabs->setCurrentWidget(s);
				return;
			}
		}
		
		SearchWidget* sw = newSearchWidget(text);
		sw->search(text,engine);
		tabs->setCurrentWidget(sw);
	}
	
	void SearchActivity::saveCurrentSearches()
	{
		QFile fptr(kt::DataDir() + "current_searches");
		if (!fptr.open(QIODevice::WriteOnly))
			return;
		
		QTextStream out(&fptr);
		foreach (SearchWidget* w,searches)
		{
			out << "TEXT: " << w->getSearchText() << ::endl;
			out << "URL: " << w->getCurrentUrl().prettyUrl() << ::endl;
			out << "SBTEXT: " << w->getSearchBarText() << ::endl;
			out << "ENGINE:" << w->getSearchBarEngine() << ::endl;
		}
	}
	
	void SearchActivity::loadCurrentSearches()
	{
		QFile fptr(kt::DataDir() + "current_searches");
		if (!fptr.open(QIODevice::ReadOnly))
			return;
		
		while (!fptr.atEnd())
		{
			QString s = QString(fptr.readLine());
			QString text,sbtext;
			int engine = 0;
			KUrl url;
		
			if (s.startsWith("TEXT:"))
				text = s.mid(5).trimmed();
			else
				continue;
			
			s = QString(fptr.readLine());
			if (!s.startsWith("URL:"))
				continue;
			
			url = KUrl(s.mid(4).trimmed());
			
			s = QString(fptr.readLine());
			if (!s.startsWith("SBTEXT:"))
				continue;
			
			sbtext = s.mid(7).trimmed();
			
			s = QString(fptr.readLine());
			if (!s.startsWith("ENGINE:"))
				continue;
			
			bool ok = false;
			engine = s.mid(7).trimmed().toInt(&ok);
			
			if (url.isValid() && ok)
			{
				SearchWidget* search = newSearchWidget(text);
				search->restore(url,text,sbtext,engine);
			}
		}
		
		if (searches.count() == 0)
		{
			SearchWidget* search = newSearchWidget(QString());
			search->home();
		}
	}
	
	void SearchActivity::find()
	{
		QWidget* w = tabs->currentWidget();
		foreach (SearchWidget* s,searches)
		{
			if (w == s)
			{
				s->find();
				break;
			}
		}
	}
	
	void SearchActivity::back()
	{
		QWidget* w = tabs->currentWidget();
		foreach (SearchWidget* s,searches)
		{
			if (w == s)
			{
				s->back();
				break;
			}
		}
	}
	
	void SearchActivity::reload()
	{
		QWidget* w = tabs->currentWidget();
		foreach (SearchWidget* s,searches)
		{
			if (w == s)
			{
				s->reload();
				break;
			}
		}
	}
	
	void SearchActivity::search()
	{
		QWidget* w = tabs->currentWidget();
		foreach (SearchWidget* s,searches)
		{
			if (w == s)
			{
				s->search();
				break;
			}
		}
	}
	
	void SearchActivity::copy()
	{
		QWidget* w = tabs->currentWidget();
		foreach (SearchWidget* s,searches)
		{
			if (w == s)
			{
				s->copy();
				break;
			}
		}
	}
	
	SearchWidget* SearchActivity::newSearchWidget(const QString & text)
	{
		SearchWidget* search = new SearchWidget(sp);
		int idx = tabs->addTab(search,KIcon("edit-find"),text);
		if (!text.isEmpty())
			tabs->setTabToolTip(idx,i18n("Search for %1",text));
		
		KAction* back_action = sp->getBackAction();
		connect(search,SIGNAL(enableBack(bool)),back_action,SLOT(setEnabled(bool)));
		connect(search,SIGNAL(openNewTab(const KUrl&)),this,SLOT(openNewTab(const KUrl&)));
		connect(search,SIGNAL(changeTitle(SearchWidget*,QString)),this,SLOT(setTabTitle(SearchWidget*,QString)));
		searches.append(search);
		search->setSearchBarEngine(sp->currentSearchEngine());
		return search;
	}
	
	void SearchActivity::openNewTab(const KUrl & url)
	{
		QString text = url.host();
		SearchWidget* search = newSearchWidget(text);
		search->restore(url,text,QString(),sp->currentSearchEngine());
		sp->getBackAction()->setEnabled(false);
		tabs->setCurrentWidget(search);
	}
	
	void SearchActivity::currentTabChanged(int idx)
	{
		sp->getBackAction()->setEnabled(false);
		foreach (SearchWidget* s,searches)
		{
			if (s == tabs->widget(idx))
			{
				sp->getBackAction()->setEnabled(s->backAvailable());
				break;
			}
		}
		
		tabs->cornerWidget(Qt::TopRightCorner)->setEnabled(searches.count() > 1);
	}
	
	void SearchActivity::home()
	{
		QWidget* w = tabs->currentWidget();
		foreach (SearchWidget* s,searches)
		{
			if (w == s)
			{
				s->home();
				break;
			}
		}
	}
	
	void SearchActivity::closeTab()
	{
		if (searches.count() == 1)
			return;
		
		foreach (SearchWidget* s,searches)
		{
			if (s == tabs->currentWidget())
			{
				tabs->removeTab(tabs->currentIndex());
				searches.removeAll(s);
				delete s;
				break;
			}
		}
	}
	
	void SearchActivity::openTab()
	{
		SearchWidget* search = newSearchWidget(QString());
		search->home();
		tabs->setCurrentWidget(search);
	}
	
	void SearchActivity::setTabTitle(SearchWidget* sw,const QString & title)
	{
		int idx = tabs->indexOf(sw);
		if (idx >= 0)
			tabs->setTabText(idx,title);
	}
}
