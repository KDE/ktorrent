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
#include <util/error.h>
#include <interfaces/functions.h>

#include <qstring.h>
#include <qtimer.h>
#include <qdatetime.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kstdaction.h>
#include <kiconloader.h>
#include <kglobal.h>

#include "scheduleeditor.h"
#include "schedule.h"
#include "bwschedulerplugin.h"


#include <torrent/globals.h>

using namespace bt;

K_EXPORT_COMPONENT_FACTORY(ktbwschedulerplugin,KGenericFactory<kt::BWSchedulerPlugin>("ktbwschedulerplugin"))

namespace kt
{	
	const QString NAME = "Bandwidth Scheduler";
	const QString AUTHOR = "Ivan Vasic";
	const QString EMAIL = "ivasic@gmail.com";
	const QString DESCRIPTION = i18n("Bandwidth scheduling plugin");

	BWSchedulerPlugin::BWSchedulerPlugin(QObject* parent, const QStringList& args)
	: Plugin(parent, NAME,i18n("Bandwidth Scheduler"),AUTHOR,EMAIL,DESCRIPTION, "clock")
	{
		setXMLFile("ktbwschedulerpluginui.rc");
		m_bws_action = 0;
		connect(&m_timer, SIGNAL(timeout()), this, SLOT(timerTriggered()));
		m_editor = 0;
	}


	BWSchedulerPlugin::~BWSchedulerPlugin()
	{
	}

	void BWSchedulerPlugin::load()
	{
		m_schedule = new Schedule();
		
		try
		{
			m_schedule->load(kt::DataDir() + "current.sched");
		}
		catch (bt::Error & err)
		{
			Out(SYS_SCD|LOG_NOTICE) << "Failed to load current.sched : " << err.toString() << endl;
			m_schedule->clear();
		}
		
		m_editor = new ScheduleEditor(0);
		connect(m_editor,SIGNAL(loaded(Schedule*)),this,SLOT(onLoaded(Schedule*)));
		getGUI()->addTabPage(m_editor,"clock",i18n("Bandwidth Schedule"),0);
		m_editor->setSchedule(m_schedule);
		
#if 0
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
#endif
	}

	void BWSchedulerPlugin::unload()
	{
		getGUI()->removeTabPage(m_editor);
		m_editor->deleteLater();
		m_editor = 0;
		
		try
		{
			m_schedule->save(kt::DataDir() + "current.sched");
		}
		catch (bt::Error & err)
		{
			Out(SYS_SCD|LOG_NOTICE) << "Failed to save current.sched : " << err.toString() << endl;
		}
		
		delete m_schedule;
		m_schedule = 0;
#if 0
		getGUI()->removePrefPage(Pref);
		if(Pref)
			delete Pref;
		Pref = 0;
		
		if(bws_action)
			delete bws_action;
		bws_action = 0;
		
		m_timer.stop();
#endif
	}
	
	void BWSchedulerPlugin::timerTriggered()
	{
		/*m_timer.changeInterval(3600*1000);
		QDateTime now = QDateTime::currentDateTime();
		BWScheduler::instance().trigger();*/
	}
	
	void BWSchedulerPlugin::openBWS()
	{
#if 0
		if(BWSchedulerPluginSettings::enableBWS())
		{
			BWSPrefPageWidget dlg;
			dlg.exec();
		}
		else
			KMessageBox::sorry(0, i18n("Bandwidth scheduler is disabled. Go to Preferences->Scheduler to enable it."));
#endif
	}
	
	void BWSchedulerPlugin::updateEnabledBWS()
	{
#if 0
		if(BWSchedulerPluginSettings::enableBWS())
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
#endif
	}
	
	void BWSchedulerPlugin::onLoaded(Schedule* ns)
	{
		delete m_schedule;
		m_schedule = ns;
		m_editor->setSchedule(ns);
	}
		
	
	bool BWSchedulerPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}
}
