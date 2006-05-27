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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstdaction.h>
#include <kpopupmenu.h>
#include <interfaces/guiinterface.h>
#include "searchplugin.h"
#include "searchwidget.h"
#include "searchprefpage.h"


#define NAME "searchplugin"
#define AUTHOR "Joris Guisson"
#define EMAIL "joris.guisson@gmail.com"



K_EXPORT_COMPONENT_FACTORY(ktsearchplugin,KGenericFactory<kt::SearchPlugin>("ktsearchplugin"))

namespace kt
{

	SearchPlugin::SearchPlugin(QObject* parent, const char* name, const QStringList& args)
	: Plugin(parent, name, args,NAME,AUTHOR,EMAIL,i18n("Search for torrents on several popular torrent search engines"))
	{
		// setXMLFile("ktsearchpluginui.rc");
		search = 0;
		pref = 0;
	}


	SearchPlugin::~SearchPlugin()
	{}


	void SearchPlugin::load()
	{
		KIconLoader* iload = KGlobal::iconLoader();
		search = new SearchWidget(this);
		getGUI()->addTabPage(
				search,iload->loadIconSet("viewmag", KIcon::Small),
				i18n("Search"));
		
		
		pref = new SearchPrefPage(this);
		getGUI()->addPrefPage(pref);

		KAction* copy_act = KStdAction::copy(search,SLOT(copy()),actionCollection());
		copy_act->plug(search->rightClickMenu(),0);
	}

	void SearchPlugin::unload()
	{
		search->onShutDown();
		getGUI()->removeTabPage(search);
		getGUI()->removePrefPage(pref);
		delete search;
		search = 0;
		delete pref;
		pref = 0;
	}
	
	void SearchPlugin::preferencesUpdated()
	{
		if(search)
			search->loadSearchEngines();
	}

}
#include "searchplugin.moc"
