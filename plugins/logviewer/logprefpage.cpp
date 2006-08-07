/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasic                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "logprefpage.h"
#include "logprefwidget.h"

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

namespace kt
{

	LogPrefPage::LogPrefPage()
		: PrefPageInterface(i18n("LogViewer"), i18n("LogViewer Options"),
						KGlobal::iconLoader()->loadIcon("toggle_log",KIcon::NoGroup))
	{
		m_widget = 0;
	}


	LogPrefPage::~LogPrefPage()
	{}
	
	bool LogPrefPage::apply()
	{
		if(m_widget)
			return m_widget->apply();
		
		return true;
	}

	void LogPrefPage::createWidget(QWidget* parent)
	{
		m_widget = new LogPrefWidget(parent);
	}

	void LogPrefPage::updateData()
	{
	}

	void LogPrefPage::deleteWidget()
	{
		if(m_widget)
			delete m_widget;
	}
}
