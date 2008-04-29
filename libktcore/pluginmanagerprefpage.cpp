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
#include <QVBoxLayout>
#include <klocale.h>
#include <kpushbutton.h>
#include <qtreewidget.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpluginselector.h>
#include <util/constants.h>
#include <util/log.h>
#include "pluginmanager.h"
#include "pluginmanagerprefpage.h"
#include "settings.h"

using namespace bt;

namespace kt
{
	PluginManagerPrefPage::PluginManagerPrefPage(PluginManager* pman) 
		: PrefPageInterface(Settings::self(),i18n("Plugins"),"kt-plugins",0),pman(pman)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		pmw = new KPluginSelector(this);
		connect(pmw,SIGNAL(changed(bool)),this,SLOT(changed()));
		connect(pmw,SIGNAL(configCommitted(const QByteArray &)),this,SLOT(changed()));
		layout->addWidget(pmw);
	}


	PluginManagerPrefPage::~PluginManagerPrefPage()
	{
	}

	void PluginManagerPrefPage::updatePluginList()
 	{
		pmw->addPlugins(pman->pluginInfoList(),KPluginSelector::IgnoreConfigFile, i18n("Plugins"));
	}
		
	void PluginManagerPrefPage::updateSettings()
	{
		pmw->updatePluginsState();
		pman->loadPlugins();
	}
	
	void PluginManagerPrefPage::changed()
	{
		updateSettings();
	}
}

#include "pluginmanagerprefpage.moc"
