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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstdaction.h>
#include <kpopupmenu.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <interfaces/guiinterface.h>
#include "searchplugin.h"
#include "searchwidget.h"
#include "searchprefpage.h"
#include "searchtab.h"
#include "searchpluginsettings.h"
#include "searchenginelist.h"


#define NAME "Search"
#define AUTHOR "Joris Guisson"
#define EMAIL "joris.guisson@gmail.com"



K_EXPORT_COMPONENT_FACTORY(ktsearchplugin,KGenericFactory<kt::SearchPlugin>("ktsearchplugin"))

namespace kt
{

	SearchPlugin::SearchPlugin(QObject* parent, const char* name, const QStringList& args)
	: Plugin(parent, name, args,NAME,i18n("Search"),AUTHOR,EMAIL,
			 i18n("Search for torrents on several popular torrent search engines"),"viewmag")
	{
		// setXMLFile("ktsearchpluginui.rc");
		pref = 0;
		tab = 0;
	}


	SearchPlugin::~SearchPlugin()
	{}


	void SearchPlugin::load()
	{
		engines.load(KGlobal::dirs()->saveLocation("data","ktorrent") + "search_engines");
		KToolBar* tb = getGUI()->addToolBar("search");
		tab = new SearchTab(tb);
		connect(tab,SIGNAL(search( const QString&, int, bool )),
				this,SLOT(search( const QString&, int, bool )));
		 
		pref = new SearchPrefPage(this);
		getGUI()->addPrefPage(pref);
		pref->updateData();
		tab->updateSearchEngines(engines);
	}

	void SearchPlugin::unload()
	{
		tab->saveSettings();
		SearchWidget* s = 0;
		while ((s = searches.first()) != 0)
		{
			getGUI()->removeTabPage(s);
			searches.removeFirst();
			delete s;
		}
		getGUI()->removeToolBar(tab->getToolBar());
		getGUI()->removePrefPage(pref);
		delete pref;
		pref = 0;
		delete tab;
		tab = 0;
	}
	
	void SearchPlugin::search(const QString & text,int engine,bool external)
	{	
		if(external)
		{
			const SearchEngineList& sl = getSearchEngineList();
		
			if (engine < 0 || engine >= sl.getNumEngines())
				engine = 0;
		
			QString s_url = sl.getSearchURL(engine).prettyURL();
			s_url.replace("FOOBAR", KURL::encode_string(text), true);
			KURL url = KURL::fromPathOrURL(s_url);
			
			if(SearchPluginSettings::useDefaultBrowser())
				kapp->invokeBrowser(url.url());
			else
				KRun::runCommand(QString("%1 \"%2\"").arg(SearchPluginSettings::customBrowser()).arg(url.url()), SearchPluginSettings::customBrowser(), "viewmag" );
			
			return;
		}
		
		KIconLoader* iload = KGlobal::iconLoader();
		
		SearchWidget* search = new SearchWidget(this);
		getGUI()->addTabPage(search,iload->loadIconSet("viewmag", KIcon::Small),text,this);
		
		KAction* copy_act = KStdAction::copy(search,SLOT(copy()),actionCollection());
		copy_act->plug(search->rightClickMenu(),0);
		searches.append(search);
		
		search->updateSearchEngines(engines);
		search->search(text,engine);
	}
	
	void SearchPlugin::preferencesUpdated()
	{
		engines.load(KGlobal::dirs()->saveLocation("data","ktorrent") + "search_engines");
		if (tab)
			tab->updateSearchEngines(engines);
		
		for (QPtrList<SearchWidget>::iterator i = searches.begin(); i != searches.end();i++)
		{
			SearchWidget* w = *i;
			w->updateSearchEngines(engines);
		}
	}
	
	void SearchPlugin::tabCloseRequest(kt::GUIInterface* gui,QWidget* tab)
	{
		if (searches.contains((SearchWidget*)tab))
		{
			searches.remove((SearchWidget*)tab);
			gui->removeTabPage(tab);
			tab->deleteLater();
		}
	}

	bool SearchPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}
}
#include "searchplugin.moc"
