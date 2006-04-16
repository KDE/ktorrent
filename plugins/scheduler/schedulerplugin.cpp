/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić                                      *
 *   ivan@ktorrent.org                                                     *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.           *
 ***************************************************************************/
#include <kgenericfactory.h>

#include <interfaces/coreinterface.h>
#include <interfaces/guiinterface.h>
#include <util/constants.h>
#include <util/log.h>

#include <qstring.h>
#include <qtimer.h>
#include <qdatetime.h>

#include <kmessagebox.h>
#include <klocale.h>

#include "schedulerplugin.h"
#include "schedulerpluginsettings.h"
#include "bwsprefpage.h"
#include "bwscheduler.h"

#include <torrent/downloadcap.h>
#include <torrent/uploadcap.h>
#include <torrent/globals.h>

using namespace bt;

K_EXPORT_COMPONENT_FACTORY(ktschedulerplugin,KGenericFactory<kt::SchedulerPlugin>("schedulerplugin"))

namespace kt
{	
	const QString NAME = "schedulerplugin";
	const QString AUTHOR = "Ivan Vasic";
	const QString EMAIL = "ivasic@gmail.com";
	const QString DESCRIPTION = i18n("KTorrent's schedule plugin");

	SchedulerPlugin::SchedulerPlugin(QObject* parent, const char* name, const QStringList& args)
	: Plugin(parent, name, args,NAME,AUTHOR,EMAIL,DESCRIPTION)
	{
		// setXMLFile("ktpluginui.rc");
		connect(&m_timer, SIGNAL(timeout()), this, SLOT(timer_triggered()));
	}


	SchedulerPlugin::~SchedulerPlugin()
	{
	}

	void SchedulerPlugin::load()
	{
		BWSPref = new BWSPrefPage();
		getGUI()->addPrefPage(BWSPref);
		BWScheduler::instance().setCoreInterface(getCore());
		
		QDateTime now = QDateTime::currentDateTime();
		QDateTime hour = now.addSecs(3600);
// 		QDateTime hour = now.addSecs(60);
		
		QTime t(hour.time().hour(), 0);
// 		QTime t(hour.time().hour(), hour.time().minute());
		
		QDateTime round(hour.date(), t);
		
		int secs_to = now.secsTo(round);
		
		m_timer.start(secs_to*1000);
		
		if(SchedulerPluginSettings::enableBWS())
			BWScheduler::instance().trigger();
	}

	void SchedulerPlugin::unload()
	{
		getGUI()->removePrefPage(BWSPref);
		delete BWSPref;
		BWSPref = 0;
		
		m_timer.stop();
	}
	
	void SchedulerPlugin::timer_triggered()
	{
		m_timer.changeInterval(60*1000);
		QDateTime now = QDateTime::currentDateTime();
		BWScheduler::instance().trigger();
	}
	
}


