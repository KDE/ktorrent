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
#include <klocale.h>
#include <kpushbutton.h>
#include <klistview.h>
#include <kglobal.h>
#include <kiconloader.h>
#include "pluginmanager.h"
#include "pluginmanagerwidget.h"
#include "pluginmanagerprefpage.h"


namespace kt
{

	PluginManagerPrefPage::PluginManagerPrefPage(PluginManager* pman)
	: PrefPageInterface(i18n("Plugins"), i18n("Plugin Options"),KGlobal::iconLoader()->loadIcon("ktplugins",KIcon::NoGroup)),pman(pman)
	{
		pmw = 0;
	}


	PluginManagerPrefPage::~PluginManagerPrefPage()
	{}

	bool PluginManagerPrefPage::apply()
	{
		return true;
	}
	
	void PluginManagerPrefPage::createWidget(QWidget* parent)
	{
		pmw = new PluginManagerWidget(parent);
		connect(pmw->load_btn,SIGNAL(clicked()),this,SLOT(onLoad()));
		connect(pmw->unload_btn,SIGNAL(clicked()),this,SLOT(onUnload()));
		connect(pmw->load_all_btn,SIGNAL(clicked()),this,SLOT(onLoadAll()));
		connect(pmw->unload_all_btn,SIGNAL(clicked()),this,SLOT(onUnloadAll()));
		updateData();
	}
	
	void PluginManagerPrefPage::updateData()
	{
		KListView* lv = pmw->plugin_view;
		lv->clear();

		// get list of plugins
		QPtrList<Plugin> pl;
		pman->fillPluginList(pl);

		QPtrList<Plugin>::iterator i = pl.begin();
		while (i != pl.end())
		{
			Plugin* p = *i;
			KListViewItem* li = new KListViewItem(lv);
			li->setText(0,p->getName());
			li->setText(1,p->isLoaded() ? i18n("Loaded") : i18n("Not loaded"));
			li->setText(2,p->getDescription());
			li->setText(3,p->getAuthor());
			i++;
		}
	}
	
	void PluginManagerPrefPage::deleteWidget()
	{
		delete pmw;
		pmw = 0;
	}

	void PluginManagerPrefPage::onLoad()
	{
		KListView* lv = pmw->plugin_view;
		QListViewItem* vi = lv->currentItem();
		if (vi && !pman->isLoaded(vi->text(0)))
		{
			pman->load(vi->text(0));
			vi->setText(1,pman->isLoaded(vi->text(0)) ? i18n("Loaded") : i18n("Not loaded"));
		}
	}
	
	void PluginManagerPrefPage::onUnload()
	{
		KListView* lv = pmw->plugin_view;
		QListViewItem* vi = lv->currentItem();
		if (vi && pman->isLoaded(vi->text(0)))
		{
			pman->unload(vi->text(0));
			vi->setText(1,pman->isLoaded(vi->text(0)) ? i18n("Loaded") : i18n("Not loaded"));
		}
	}
	
	void PluginManagerPrefPage::onLoadAll()
	{
		pman->loadAll();
		updateData();
	}
	
	void PluginManagerPrefPage::onUnloadAll()
	{
		pman->unloadAll();
		updateData();
	}
		
}
