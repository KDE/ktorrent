/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include "ipblockingprefpage.h"
#include "ipblockingpref.h"
#include "ipfilterpluginsettings.h"
#include "ipfilterplugin.h"
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kurlrequester.h>
#include <util/log.h>
#include <torrent/globals.h>
#include <interfaces/coreinterface.h>
#include <qthread.h>

using namespace bt;

namespace kt
{

	LoadingThread::LoadingThread(CoreInterface* core) : QThread()
	{
		m_core = core;
		this->start(QThread::LowPriority);
	}
	
	LoadingThread::~ LoadingThread()
	{}
	
	void LoadingThread::run()
	{
		QString filter = IPBlockingPluginSettings::filterFile();
		if(!filter.isEmpty())
		{
			//load list
			QString listURL = IPBlockingPluginSettings::filterFile();
			QFile dat(listURL);
			dat.open(IO_ReadOnly);

			QString trt;
			QTextStream stream( &dat );
			QString line;
			int i=0;
			while ( !stream.atEnd() )
			{
				line = stream.readLine();
				m_core->addBlockedIP(line);
				++i;
			}
			Out() << "Loaded " << i << " blocked IP ranges." << endl;
			dat.close();
		}
		delete this;
	}
	
	IPBlockingPrefPageWidget::IPBlockingPrefPageWidget(QWidget* parent) : IPBlockingPref(parent)
	{
		m_filter->setURL(IPBlockingPluginSettings::filterFile());
	}

	void IPBlockingPrefPageWidget::apply()
	{
		KURLRequester* filter = m_filter;
		IPBlockingPluginSettings::setFilterFile(filter->url());
		IPBlockingPluginSettings::writeConfig();
	}

	IPBlockingPrefPage::IPBlockingPrefPage(CoreInterface* core)
	: PrefPageInterface(i18n("IPBlocking filter"), i18n("IPBlocking filter Options"), KGlobal::iconLoader()->loadIcon("filter",KIcon::NoGroup)), m_core(core)
	{
		widget = 0;
	}

	IPBlockingPrefPage::~IPBlockingPrefPage()
	{}

	void IPBlockingPrefPage::apply()
	{
		widget->apply();
		LoadingThread* filters = new LoadingThread(this->m_core);
	}
	
	void IPBlockingPrefPage::loadFilters()
	{
		LoadingThread* filters = new LoadingThread(this->m_core);
	}

	void IPBlockingPrefPage::createWidget(QWidget* parent)
	{
		widget = new IPBlockingPrefPageWidget(parent);
	}

	void IPBlockingPrefPage::updateData()
	{}
}
