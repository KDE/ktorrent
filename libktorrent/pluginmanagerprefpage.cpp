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
#include <klocale.h>
#include <kpushbutton.h>
#include <qtreewidget.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <util/constants.h>
#include "pluginmanager.h"
#include "ui_pluginmanagerwidget.h"
#include "pluginmanagerprefpage.h"
#include "labelview.h"
#include "settings.h"

using namespace bt;

namespace kt
{
	class PluginViewItem : public LabelViewItem
	{
		Plugin* p;
	public:
		PluginViewItem(Plugin* p,LabelView* parent) 
			: LabelViewItem(p->getIcon(),p->getName(),p->getDescription(),parent),p(p)
		{
			update();
		}
		
		virtual ~PluginViewItem()
		{}
		
		virtual void update()
		{
			setTitle("<h3>" + p->getGuiName() + "</h3>");
			setDescription(i18n("%1<br>Status: <b>%2</b><br>Author: %3",
						p->getDescription(),p->isLoaded() ? i18n("Loaded") : i18n("Not loaded"),p->getAuthor()));
		}
		
		QString pluginName() {return p->getName();}
	};


	PluginManagerPrefPage::PluginManagerPrefPage(PluginManager* pman) 
		: PrefPageInterface(Settings::self(),i18n("Plugins"),"ktplugins",0),pman(pman)
	{
		pmw = new Ui_PluginManagerWidget();
		pmw->setupUi(this);
		connect(pmw->load_btn,SIGNAL(clicked()),this,SLOT(onLoad()));
		connect(pmw->unload_btn,SIGNAL(clicked()),this,SLOT(onUnload()));
		connect(pmw->load_all_btn,SIGNAL(clicked()),this,SLOT(onLoadAll()));
		connect(pmw->unload_all_btn,SIGNAL(clicked()),this,SLOT(onUnloadAll()));
		LabelView* lv = pmw->plugin_view;
		connect(lv,SIGNAL(currentChanged(LabelViewItem * )),this,SLOT(onCurrentChanged( LabelViewItem* )));
	}


	PluginManagerPrefPage::~PluginManagerPrefPage()
	{
		delete pmw;
	}

	void PluginManagerPrefPage::updatePluginList()
 	{
		LabelView* lv = pmw->plugin_view;
 		lv->clear();
 		
		// get list of plugins
 		QList<Plugin*> pl;
 		pman->fillPluginList(pl);
 		
		// Add them all
 		QList<Plugin*>::iterator i = pl.begin();
 		while (i != pl.end())
 		{
 			Plugin* p = *i;
			lv->addItem(new PluginViewItem(p,lv));
 			i++;
 		}		
		lv->sort();
	}
	
	void PluginManagerPrefPage::onCurrentChanged(LabelViewItem* item)
	{
		PluginViewItem* pvi = (PluginViewItem*)item;
		if (!item)
		{
			pmw->load_btn->setEnabled(false);
			pmw->unload_btn->setEnabled(false);
		}
		else
		{
			bool loaded = pman->isLoaded(pvi->pluginName());
			pmw->load_btn->setEnabled(!loaded);
			pmw->unload_btn->setEnabled(loaded);
		}
	//	onCurrentChanged(pmw->plugin_view->selectedItem());
	}
	
	void PluginManagerPrefPage::updateAllButtons()
	{
		Uint32 tot = 0;
		Uint32 loaded = 0;
		// get list of plugins
		QList<Plugin*> pl;
		pman->fillPluginList(pl);
		
		QList<Plugin*>::iterator i = pl.begin();
		while (i != pl.end())
		{
			Plugin* p = *i;
			tot++;
			if (p->isLoaded())
				loaded++;
			i++;
		}
		
		if (loaded == tot)
		{
			pmw->load_all_btn->setEnabled(false);
			pmw->unload_all_btn->setEnabled(true);
		}
		else if (loaded < tot && loaded > 0)
		{
			pmw->unload_all_btn->setEnabled(true);
			pmw->load_all_btn->setEnabled(true);
		}
		else
		{
			pmw->unload_all_btn->setEnabled(false);
			pmw->load_all_btn->setEnabled(true);
		}
		onCurrentChanged(pmw->plugin_view->selectedItem());
	}

	void PluginManagerPrefPage::onLoad()
	{
		LabelView* lv = pmw->plugin_view;
		PluginViewItem* vi = (PluginViewItem*)lv->selectedItem();
		if (vi && !pman->isLoaded(vi->pluginName()))
		{
			pman->load(vi->pluginName());
			vi->update();
			updateAllButtons();
		}
	}
	
	void PluginManagerPrefPage::onUnload()
	{
		LabelView* lv = pmw->plugin_view;
		PluginViewItem* vi = (PluginViewItem*)lv->selectedItem();
		if (vi && pman->isLoaded(vi->pluginName()))
		{
			pman->unload(vi->pluginName());
			vi->update();
			updateAllButtons();
		}
	}
	
	void PluginManagerPrefPage::onLoadAll()
	{
		pman->loadAll();
		LabelView* lv = pmw->plugin_view;
		lv->update();
		updateAllButtons();
	}
	
	void PluginManagerPrefPage::onUnloadAll()
	{
		pman->unloadAll();
		LabelView* lv = pmw->plugin_view;
		lv->update();
		updateAllButtons();
	}
		
}

#include "pluginmanagerprefpage.moc"
