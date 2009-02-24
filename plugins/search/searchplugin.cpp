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
#include "searchactivity.h"

K_EXPORT_COMPONENT_FACTORY(ktsearchplugin,KGenericFactory<kt::SearchPlugin>("ktsearchplugin"))
		
using namespace bt;

namespace kt
{

	SearchPlugin::SearchPlugin(QObject* parent, const QStringList& args) : Plugin(parent),engines(0)
	{
		Q_UNUSED(args);
		pref = 0;
		toolbar = 0;
		setXMLFile("ktsearchpluginui.rc");
	}


	SearchPlugin::~SearchPlugin()
	{}


	void SearchPlugin::load()
	{
		LogSystemManager::instance().registerSystem(i18n("Search"),SYS_SRC);
		engines = new SearchEngineList(kt::DataDir() + "searchengines/");
		engines->loadEngines();
	
		toolbar = new SearchToolBar(this,engines);
		
		connect(toolbar,SIGNAL(search( const QString&, int, bool )),
				this,SLOT(search( const QString&, int, bool )));
		 
		pref = new SearchPrefPage(this,engines,0);
		getGUI()->addPrefPage(pref);
		connect(getCore(),SIGNAL(settingsChanged()),this,SLOT(preferencesUpdated()));
		connect(pref,SIGNAL(clearSearchHistory()),toolbar,SLOT(clearHistory()));
		
		activity = new SearchActivity(this,0);
		getGUI()->addActivity(activity);
		setupActions();
		activity->loadCurrentSearches();
	}

	void SearchPlugin::unload()
	{
		LogSystemManager::instance().unregisterSystem(i18n("Search"));
		getGUI()->removeActivity(activity);
		activity->saveCurrentSearches();
		toolbar->saveSettings();
		toolbar->deleteLater();
		
		getGUI()->removePrefPage(pref);
		pref = 0;
		toolbar = 0;
		disconnect(getCore(),SIGNAL(settingsChanged()),this,SLOT(preferencesUpdated()));
		delete engines;
		engines = 0;
		delete activity;
		activity = 0;
	}
	
	void SearchPlugin::search(const QString & text,int engine,bool external)
	{	
		if (external)
		{
			if (engine < 0 || engine >= (int)engines->getNumEngines())
				engine = 0;
		
			KUrl url = engines->search(engine,text);
			
			if(SearchPluginSettings::useDefaultBrowser())
				KRun::runUrl(url,"text/html",0);
			else
				KRun::runCommand(QString("%1 %2").arg(SearchPluginSettings::customBrowser()).arg(KShell::quoteArg(url.url())),0);
		}
		else
		{
			activity->search(text,engine);
			getGUI()->setCurrentActivity(activity);
		}
	}
	
	void SearchPlugin::preferencesUpdated()
	{
	}

	bool SearchPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}
	
	void SearchPlugin::setupActions()
	{
		KActionCollection* ac = actionCollection();
		back_action = KStandardAction::back(activity,SLOT(back()),this);
		ac->addAction("search_tab_back",back_action);
		
		reload_action = KStandardAction::redisplay(activity,SLOT(reload()),this);
		ac->addAction("search_tab_reload",reload_action);
		
		search_action = new KAction(KIcon("edit-find"),i18n("Search"),this);
		connect(search_action,SIGNAL(triggered()),activity,SLOT(search()));
		ac->addAction("search_tab_search",search_action);
		
		find_action = KStandardAction::find(activity,SLOT(find()),this);
		ac->addAction("search_tab_find",find_action);
		
		copy_action = KStandardAction::copy(activity,SLOT(copy()),this);
		ac->addAction("search_tab_copy",copy_action);
		
		home_action = KStandardAction::home(activity,SLOT(home()),this);
		ac->addAction("search_home",home_action);
	}
	
	int SearchPlugin::currentSearchEngine() const
	{
		return toolbar->currentSearchEngine();
	}
}
#include "searchplugin.moc"
