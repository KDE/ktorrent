/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kactioncollection.h>
#include <kstdaction.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <kmenu.h>
#include <kshell.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include <interfaces/functions.h>
#include <util/log.h>
#include <util/logsystemmanager.h>
#include "searchplugin.h"
#include "searchwidget.h"
#include "searchprefpage.h"
#include "searchtoolbar.h"
#include "searchpluginsettings.h"
#include "searchenginelist.h"

K_EXPORT_COMPONENT_FACTORY(ktsearchplugin,KGenericFactory<kt::SearchPlugin>("ktsearchplugin"))
		
using namespace bt;

namespace kt
{

	SearchPlugin::SearchPlugin(QObject* parent, const QStringList& args) : Plugin(parent)
	{
		Q_UNUSED(args);
		pref = 0;
		toolbar = 0;
		setupActions();
		setXMLFile("ktsearchpluginui.rc");
	}


	SearchPlugin::~SearchPlugin()
	{}


	void SearchPlugin::load()
	{
		LogSystemManager::instance().registerSystem(i18n("Search"),SYS_SRC);
		getGUI()->addCurrentTabPageListener(this);
		engines.load(kt::DataDir() + "search_engines");
		toolbar = new SearchToolBar(this);
		
		connect(toolbar,SIGNAL(search( const QString&, int, bool )),
				this,SLOT(search( const QString&, int, bool )));
		 
		pref = new SearchPrefPage(this,0);
		getGUI()->addPrefPage(pref);
		toolbar->updateSearchEngines(engines);
		connect(getCore(),SIGNAL(settingsChanged()),this,SLOT(preferencesUpdated()));
		connect(pref,SIGNAL(clearSearchHistory()),toolbar,SLOT(clearHistory()));
		connect(pref,SIGNAL(engineListUpdated()),this,SLOT(preferencesUpdated()));
		loadCurrentSearches();
	}

	void SearchPlugin::unload()
	{
		LogSystemManager::instance().unregisterSystem(i18n("Search"));
		getGUI()->removeCurrentTabPageListener(this);
		saveCurrentSearches();
		toolbar->saveSettings();
		toolbar->deleteLater();

		foreach (SearchWidget* s,searches)
		{
			getGUI()->removeTabPage(s);
			s->deleteLater();
		}
		searches.clear();
		
		getGUI()->removePrefPage(pref);
		pref = 0;
		toolbar = 0;
		disconnect(getCore(),SIGNAL(settingsChanged()),this,SLOT(preferencesUpdated()));
	}
	
	void SearchPlugin::search(const QString & text,int engine,bool external)
	{	
		if(external)
		{
			const SearchEngineList& sl = getSearchEngineList();
		
			if (engine < 0 || engine >= sl.getNumEngines())
				engine = 0;
		
			QString s_url = sl.getSearchURL(engine).prettyUrl();
			s_url.replace("FOOBAR",  QUrl::toPercentEncoding(text), Qt::CaseSensitive);
			KUrl url = KUrl(s_url);
			
			if(SearchPluginSettings::useDefaultBrowser())
				KRun::runUrl(url,"text/html",0);
			else
				KRun::runCommand(QString("%1 %2")
                                        .arg(SearchPluginSettings::customBrowser()).arg(KShell::quoteArg(url.url())),0);
			
			return;
		}
		
		
		SearchWidget* search = new SearchWidget(this);
		getGUI()->addTabPage(search,"edit-find",text,this);
		
		connect(search,SIGNAL(enableBack(bool)),back_action,SLOT(setEnabled(bool)));
		connect(search,SIGNAL(openNewTab(const KUrl&)),this,SLOT(openNewTab(const KUrl&)));
		searches.append(search);
		back_action->setEnabled(false);
		
		search->updateSearchEngines(engines);
		search->search(text,engine);
	}
	
	void SearchPlugin::preferencesUpdated()
	{
		engines.load(kt::DataDir() + "search_engines");
		if (toolbar)
			toolbar->updateSearchEngines(engines);
		
		foreach (SearchWidget* w,searches)
			w->updateSearchEngines(engines);
	}
	
	void SearchPlugin::tabCloseRequest(kt::GUIInterface* gui,QWidget* tab)
	{
		if (searches.contains((SearchWidget*)tab))
		{
			searches.removeAll((SearchWidget*)tab);
			gui->removeTabPage(tab);
			tab->deleteLater();
		}
	}

	bool SearchPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}
	
	void SearchPlugin::saveCurrentSearches()
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
	
	void SearchPlugin::loadCurrentSearches()
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
				SearchWidget* search = new SearchWidget(this);
				getGUI()->addTabPage(search,"edit-find",text,this);
		
				connect(search,SIGNAL(enableBack(bool)),back_action,SLOT(setEnabled(bool)));
				connect(search,SIGNAL(openNewTab(const KUrl&)),this,SLOT(openNewTab(const KUrl&)));
				
				searches.append(search);
		
				search->updateSearchEngines(engines);
				search->restore(url,text,sbtext,engine);
				back_action->setEnabled(false);
			}
		}
	}
	
	void SearchPlugin::setupActions()
	{
		KActionCollection* ac = actionCollection();
		back_action = KStandardAction::back(this,SLOT(back()),this);
		ac->addAction("search_tab_back",back_action);
		
		reload_action = KStandardAction::redisplay(this,SLOT(reload()),this);
		ac->addAction("search_tab_reload",reload_action);
		
		search_action = new KAction(KIcon("edit-find"),i18n("Search"),this);
		connect(search_action,SIGNAL(triggered()),this,SLOT(search()));
		ac->addAction("search_tab_search",search_action);
		
		find_action = KStandardAction::find(this,SLOT(find()),this);
		ac->addAction("search_tab_find",find_action);
		
		copy_action = KStandardAction::copy(this,SLOT(copy()),this);
		ac->addAction("search_tab_copy",copy_action);
	}
	
	void SearchPlugin::find()
	{
		QWidget* w = getGUI()->getCurrentTab();
		foreach (SearchWidget* s,searches)
		{
			if (w == s)
			{
				s->find();
				break;
			}
		}
	}
	
	void SearchPlugin::back()
	{
		QWidget* w = getGUI()->getCurrentTab();
		foreach (SearchWidget* s,searches)
		{
			if (w == s)
			{
				s->back();
				break;
			}
		}
	}
	
	void SearchPlugin::reload()
	{
		QWidget* w = getGUI()->getCurrentTab();
		foreach (SearchWidget* s,searches)
		{
			if (w == s)
			{
				s->reload();
				break;
			}
		}
	}
	
	void SearchPlugin::search()
	{
		QWidget* w = getGUI()->getCurrentTab();
		foreach (SearchWidget* s,searches)
		{
			if (w == s)
			{
				s->search();
				break;
			}
		}
	}
	
	void SearchPlugin::copy()
	{
		QWidget* w = getGUI()->getCurrentTab();
		foreach (SearchWidget* s,searches)
		{
			if (w == s)
			{
				s->copy();
				break;
			}
		}
	}
	
	void SearchPlugin::openNewTab(const KUrl & url)
	{
		SearchWidget* search = new SearchWidget(this);
		QString text = url.host();
		getGUI()->addTabPage(search,"edit-find",text,this);
		
		connect(search,SIGNAL(enableBack(bool)),back_action,SLOT(setEnabled(bool)));
		connect(search,SIGNAL(openNewTab(const KUrl&)),this,SLOT(openNewTab(const KUrl&)));
				
		searches.append(search);
		
		search->updateSearchEngines(engines);
		search->restore(url,text,QString(),toolbar->currentSearchEngine());
		back_action->setEnabled(false);
		getGUI()->setCurrentTab(search);
	}
	
	void SearchPlugin::currentTabPageChanged(QWidget* page)
	{
		back_action->setEnabled(false);
		QWidget* w = getGUI()->getCurrentTab();
		foreach (SearchWidget* s,searches)
		{
			if (s == page)
			{
				back_action->setEnabled(s->backAvailable());
				break;
			}
		}
	}
}
#include "searchplugin.moc"
