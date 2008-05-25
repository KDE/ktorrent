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
#include <kstdaction.h>
#include <kiconloader.h>
#include <kglobal.h>

#include "schedulerplugin.h"
#include "schedulerpluginsettings.h"
#include "bwscheduler.h"
#include "schedulerprefpage.h"
#include "bwsprefpagewidget.h"

#include <torrent/downloadcap.h>
#include <torrent/uploadcap.h>
#include <torrent/globals.h>

using namespace bt;

K_EXPORT_COMPONENT_FACTORY(ktschedulerplugin,KGenericFactory<kt::SchedulerPlugin>("schedulerplugin"))

namespace kt
{	
	const QString NAME = "Bandwith Scheduler";
	const QString AUTHOR = "Ivan Vasic";
	const QString EMAIL = "ivasic@gmail.com";
	const QString DESCRIPTION = i18n("Bandwidth scheduling plugin");

	SchedulerPlugin::SchedulerPlugin(QObject* parent, const char* name, const QStringList& args)
	: Plugin(parent, name, args,NAME,i18n("Bandwidth Scheduler"),AUTHOR,EMAIL,DESCRIPTION, "clock")
	{
		setXMLFile("ktschedulerpluginui.rc");
		bws_action = 0;
		connect(&m_timer, SIGNAL(timeout()), this, SLOT(timer_triggered()));
	}


	SchedulerPlugin::~SchedulerPlugin()
	{
	}

	void SchedulerPlugin::load()
	{
		Pref = new SchedulerPrefPage(this);
		getGUI()->addPrefPage(Pref);
		BWScheduler::instance().setCoreInterface(getCore());
		
		QDateTime now = QDateTime::currentDateTime();
		
		//each hour
		QDateTime hour = now.addSecs(3600);
		QTime t(hour.time().hour(), 0);
		
		//each minute
// 		QDateTime hour = now.addSecs(60);
// 		QTime t(hour.time().hour(), hour.time().minute());
		
		QDateTime round(hour.date(), t);
		
		// add a 5 second safety margin (BUG: 131246)
		int secs_to = now.secsTo(round) + 5;
		
		m_timer.start(secs_to*1000);

		BWScheduler::instance().trigger();
		
// 		updateEnabledBWS();
		bws_action = new KAction(i18n("Open Bandwidth Scheduler" ), "clock", 0, this,
								 SLOT(openBWS()), actionCollection(), "bwscheduler" );
	}

	void SchedulerPlugin::unload()
	{
		getGUI()->removePrefPage(Pref);
		if(Pref)
			delete Pref;
		Pref = 0;
		
		if(bws_action)
			delete bws_action;
		bws_action = 0;
		
		m_timer.stop();
	}
	
	void SchedulerPlugin::timer_triggered()
	{
		m_timer.changeInterval(3600*1000);
		QDateTime now = QDateTime::currentDateTime();
		BWScheduler::instance().trigger();
	}
	
	void SchedulerPlugin::openBWS()
	{
		if(SchedulerPluginSettings::enableBWS())
		{
			BWSPrefPageWidget dlg;
			dlg.exec();
		}
		else
			KMessageBox::sorry(0, i18n("Bandwidth scheduler is disabled. Go to Preferences->Scheduler to enable it."));
	}
	
	void SchedulerPlugin::updateEnabledBWS()
	{
		if(SchedulerPluginSettings::enableBWS())
		{
			bws_action = new KAction(i18n("Open Bandwidth Scheduler" ), "clock", 0, this,
									 SLOT(openBWS()), actionCollection(), "bwscheduler" );
		}
		else
		{
			if(bws_action)
				delete bws_action;
			bws_action = 0;
		}
	}
	
	bool SchedulerPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}
}
