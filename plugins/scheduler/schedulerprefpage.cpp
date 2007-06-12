/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić                                      *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.           *
 ***************************************************************************/

#include "schedulerprefpage.h"
#include "bwsprefpagewidget.h"
#include "schedulerplugin.h"
#include "schedulerpluginsettings.h"

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>


namespace kt
{

	SchedulerPrefPage::SchedulerPrefPage(SchedulerPlugin* plugin)
		: PrefPageInterface(i18n("Scheduler"), i18n("Scheduler plugin options"), KGlobal::iconLoader()->loadIcon("clock",KIcon::NoGroup)), m_plugin(plugin)
	{
		widget = 0;
	}


	SchedulerPrefPage::~SchedulerPrefPage()
	{}

	bool SchedulerPrefPage::apply()
	{
		widget->apply();
//		m_plugin->updateEnabledBWS();
		return true;
	}

	void SchedulerPrefPage::createWidget( QWidget * parent )
	{
		widget = new SchedulerPrefPageWidget(parent);
	}

	void SchedulerPrefPage::updateData()
	{}

	void SchedulerPrefPage::deleteWidget()
	{
		delete widget;
		widget = 0;
	}
}
