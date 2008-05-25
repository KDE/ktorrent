   /***************************************************************************
 *   Copyright (C) 2006 by Diego R. Brogna                                 *
 *   dierbro@gmail.com                                               	   *
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

#include "webinterfaceprefpage.h"
#include "webinterfaceplugin.h"
namespace kt
{

	WebInterfacePrefPage::WebInterfacePrefPage(WebInterfacePlugin* plugin)
		: PrefPageInterface(i18n("WebInterface"), i18n("WebInterface Options"),
						KGlobal::iconLoader()->loadIcon("toggle_log",KIcon::NoGroup))
	{
		m_widget = 0;
		w_plugin=plugin;
	}


	WebInterfacePrefPage::~WebInterfacePrefPage()
	{}
	
	bool WebInterfacePrefPage::apply()
	{
		if(m_widget)
			m_widget->apply();
		w_plugin->preferencesUpdated();
		return true;
	}

	void WebInterfacePrefPage::createWidget(QWidget* parent)
	{
		m_widget = new WebInterfacePrefWidget(parent);
	}

	void WebInterfacePrefPage::updateData()
	{
	}

	void WebInterfacePrefPage::deleteWidget()
	{
		if(m_widget)
			delete m_widget;
	}
}
