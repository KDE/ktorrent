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
#include "searchactivity.h"

#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>
#include <QToolButton>
#include <KLocale>
#include <KIcon>
#include <KConfigGroup>
#include <KStandardAction>
#include <KActionCollection>
#include <interfaces/functions.h>
#include <util/indexofcompare.h>
#include <util/error.h>
#include <bcodec/bencoder.h>
#include <bcodec/bdecoder.h>
#include <bcodec/bnode.h>

#include "searchwidget.h"
#include "searchplugin.h"
#include <searchpluginsettings.h>
#include "searchtoolbar.h"


namespace kt
{
	SearchActivity::SearchActivity(SearchPlugin* sp,QWidget* parent)
		: Activity(i18nc("plugin name","Search"),"edit-find",10,parent),sp(sp)
	{
		setXMLGUIFile("ktsearchpluginui.rc");
		setupActions();
		toolbar = new SearchToolBar(part()->actionCollection(),sp->getSearchEngineList(),this);
		connect(toolbar,SIGNAL(search( const QString&, int, bool )),
				sp,SLOT(search( const QString&, int, bool )));
		
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		tabs = new KTabWidget(this);
		tabs->setMovable(true);
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
	
	void SearchActivity::setupActions()
	{
		KActionCollection* ac = part()->actionCollection();
		
		search_action = new KAction(KIcon("edit-find"),i18n("Search"),this);
		connect(search_action,SIGNAL(triggered()),this,SLOT(search()));
		ac->addAction("search_tab_search",search_action);
		
		find_action = KStandardAction::find(this,SLOT(find()),this);
		ac->addAction("search_tab_find",find_action);
		
		home_action = KStandardAction::home(this,SLOT(home()),this);
		ac->addAction("search_home",home_action);
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
		
		// Sort by order in tab widget so that they are restored in the proper order
		qSort(searches.begin(),searches.end(),IndexOfCompare<KTabWidget,SearchWidget>(tabs));
		bt::BEncoder enc(&fptr);
		enc.beginList();
		foreach (SearchWidget* w,searches)
		{
			enc.beginDict();
			enc.write("TEXT",w->getSearchText());
			enc.write("URL",w->getCurrentUrl().prettyUrl());
			enc.write("SBTEXT",w->getSearchBarText());
			enc.write("ENGINE",(bt::Uint32)w->getSearchBarEngine());
			enc.end();
		}
		enc.end();
	}
	
	void SearchActivity::loadCurrentSearches()
	{
		if (!SearchPluginSettings::restorePreviousSession())
		{
			SearchWidget* search = newSearchWidget(QString());
			search->home();
			return;
		}
		
		QFile fptr(kt::DataDir() + "current_searches");
		if (!fptr.open(QIODevice::ReadOnly))
		{
			SearchWidget* search = newSearchWidget(QString());
			search->home();
			return;
		}
		
		QByteArray data = fptr.readAll();
		bt::BDecoder dec(data,false,0);
		bt::BListNode* search_list = 0;
		try
		{
			search_list = dec.decodeList();
			if (!search_list) 
				throw bt::Error("Invalid current searches");

			for (bt::Uint32 i = 0;i < search_list->getNumChildren();i++)
			{
				bt::BDictNode* dict = search_list->getDict(i);
				if (!dict)
					continue;
				
				QString text = dict->getString("TEXT",0);
				QString sbtext = dict->getString("SBTEXT",0);
				int engine = dict->getInt("ENGINE");
				KUrl url = dict->getString("URL",0);
				
				SearchWidget* search = newSearchWidget(text);
				search->restore(url,text,sbtext,engine);
			}
			
			delete search_list;
		}
		catch (...)
		{
			delete search_list;
		}
	
		if (searches.count() == 0)
		{
			SearchWidget* search = newSearchWidget(QString());
			search->home();
		}
	}
	
	void SearchActivity::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("SearchActivity");
		g.writeEntry("current_search",tabs->currentIndex());
		toolbar->saveSettings();
	}

	void SearchActivity::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("SearchActivity");
		int idx = g.readEntry("current_search",0);
		tabs->setCurrentIndex(idx);
	}

	void SearchActivity::find()
	{
		QWidget* w = tabs->currentWidget();
		foreach (SearchWidget* s,searches)
		{
			if (w == s)
			{
//				s->find();
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
	
	/*
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
	*/
	
	SearchWidget* SearchActivity::newSearchWidget(const QString & text)
	{
		SearchWidget* search = new SearchWidget(sp);
		int idx = tabs->addTab(search,KIcon("edit-find"),text);
		if (!text.isEmpty())
			tabs->setTabToolTip(idx,i18n("Search for %1",text));
		
		connect(search,SIGNAL(openNewTab(const KUrl&)),this,SLOT(openNewTab(const KUrl&)));
		connect(search,SIGNAL(changeTitle(SearchWidget*,QString)),this,SLOT(setTabTitle(SearchWidget*,QString)));
		connect(search,SIGNAL(changeIcon(SearchWidget*,QIcon)),this,SLOT(setTabIcon(SearchWidget*,QIcon)));
		searches.append(search);
		search->setSearchBarEngine(toolbar->currentSearchEngine());
		return search;
	}
	
	SearchWidget* SearchActivity::newTab()
	{
		return newSearchWidget(QString());
	}

	
	void SearchActivity::openNewTab(const KUrl & url)
	{
		QString text = url.host();
		SearchWidget* search = newSearchWidget(text);
		search->restore(url,text,QString(),toolbar->currentSearchEngine());
		tabs->setCurrentWidget(search);
	}
	
	void SearchActivity::currentTabChanged(int idx)
	{
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
	
	void SearchActivity::setTabIcon(SearchWidget* sw, const QIcon& icon)
	{
		int idx = tabs->indexOf(sw);
		if (idx >= 0)
			tabs->setTabIcon(idx,icon);
	}

	
	void SearchActivity::clearSearchHistory()
	{
		toolbar->clearHistory();
	}

}
