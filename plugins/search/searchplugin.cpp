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



K_EXPORT_COMPONENT_FACTORY(ktsearchplugin,KGenericFactory<kt::SearchPlugin>("ktsearchplugin"))

namespace kt
{
	const QString NAME = "searchplugin";
	const QString AUTHOR = "Joris Guisson";
	const QString EMAIL = "joris.guisson@gmail.com";
	const QString DESCRIPTION = "KTorrent's search plugin";

	SearchPlugin::SearchPlugin(QObject* parent, const char* name, const QStringList& args)
	: Plugin(parent, name, args,NAME,AUTHOR,EMAIL,DESCRIPTION)
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
		search->loadSearchEngines();
		
		pref = new SearchPrefPage();
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

}
#include "searchplugin.moc"
