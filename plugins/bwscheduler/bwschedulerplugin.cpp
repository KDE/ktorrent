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
#include <ktoolbar.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kmainwindow.h>
#include <ktoggleaction.h>

#include <interfaces/coreinterface.h>
#include <interfaces/guiinterface.h>
#include <util/constants.h>
#include <util/log.h>
#include <util/logsystemmanager.h>
#include <util/error.h>
#include <net/socketmonitor.h>
#include <interfaces/functions.h>
#include <settings.h>

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
#include "bwprefpage.h"


#include <torrent/globals.h>
#include <peer/peermanager.h>

using namespace bt;

K_EXPORT_COMPONENT_FACTORY(ktbwschedulerplugin,KGenericFactory<kt::BWSchedulerPlugin>("ktbwschedulerplugin"))

namespace kt
{	

	BWSchedulerPlugin::BWSchedulerPlugin(QObject* parent, const QStringList& args) : Plugin(parent)
	{
		Q_UNUSED(args);
		connect(&m_timer, SIGNAL(timeout()), this, SLOT(timerTriggered()));
		m_editor = 0;
		m_pref = 0;
	}


	BWSchedulerPlugin::~BWSchedulerPlugin()
	{
	}

	void BWSchedulerPlugin::load()
	{
		LogSystemManager::instance().registerSystem(i18n("Bandwidth Scheduler"),SYS_SCD);
		m_schedule = new Schedule();
		m_pref = new BWPrefPage(0);
		connect(m_pref,SIGNAL(colorsChanged()),this,SLOT(colorsChanged()));
		getGUI()->addPrefPage(m_pref);
		
		connect(getCore(),SIGNAL(settingsChanged()),this,SLOT(colorsChanged()));
		
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
		connect(m_editor,SIGNAL(scheduleChanged()),this,SLOT(timerTriggered()));
		getGUI()->addActivity(m_editor);
		m_editor->setSchedule(m_schedule);
		
		// make sure that schedule gets applied again if the settings change
		connect(getCore(),SIGNAL(settingsChanged()),this,SLOT(timerTriggered()));
		timerTriggered();
	}

	void BWSchedulerPlugin::unload()
	{
		LogSystemManager::instance().unregisterSystem(i18n("Bandwidth Scheduler"));
		m_timer.stop();
	
		getGUI()->removeActivity(m_editor);
		delete m_editor;
		m_editor = 0;
		
		getGUI()->removePrefPage(m_pref);
		m_pref = 0;
		
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
	}
	
	void BWSchedulerPlugin::timerTriggered()
	{
		QDateTime now = QDateTime::currentDateTime();
		ScheduleItem* item = m_schedule->getCurrentItem(now);
		if (!item)
		{
			Out(SYS_SCD|LOG_NOTICE) << QString("Changing schedule to normal values : %1 down, %2 up")
					.arg(Settings::maxDownloadRate()).arg(Settings::maxUploadRate()) << endl;
			// set normal limits
			getCore()->setPausedState(false);
			net::SocketMonitor::setDownloadCap(1024 * Settings::maxDownloadRate());
			net::SocketMonitor::setUploadCap(1024 * Settings::maxUploadRate());
			if (m_editor)
				m_editor->updateStatusText(Settings::maxUploadRate(),Settings::maxDownloadRate(),false);
			
			PeerManager::setMaxConnections(Settings::maxConnections());
			PeerManager::setMaxTotalConnections(Settings::maxTotalConnections());
		}
		else if (item->paused)
		{
			Out(SYS_SCD|LOG_NOTICE) << QString("Changing schedule to : PAUSED") << endl;
			if (!getCore()->getPausedState())
			{
				getCore()->setPausedState(true);
				net::SocketMonitor::setDownloadCap(1024 * Settings::maxDownloadRate());
				net::SocketMonitor::setUploadCap(1024 * Settings::maxUploadRate());
				if (m_editor)
					m_editor->updateStatusText(Settings::maxUploadRate(),Settings::maxDownloadRate(),true);
			}
			
			if (item->set_conn_limits)
			{
				Out(SYS_SCD|LOG_NOTICE) << QString("Setting connection limits to : %1 per torrent, %2 global")
						.arg(item->torrent_conn_limit).arg(item->global_conn_limit) << endl;
				PeerManager::setMaxConnections(item->torrent_conn_limit);
				PeerManager::setMaxTotalConnections(item->global_conn_limit);
			}
			else
			{
				PeerManager::setMaxConnections(Settings::maxConnections());
				PeerManager::setMaxTotalConnections(Settings::maxTotalConnections());
			}
		}
		else
		{
			Out(SYS_SCD|LOG_NOTICE) << QString("Changing schedule to : %1 down, %2 up")
					.arg(item->download_limit).arg(item->upload_limit) << endl;
			getCore()->setPausedState(false);
			net::SocketMonitor::setDownloadCap(1024 * item->download_limit);
			net::SocketMonitor::setUploadCap(1024 * item->upload_limit);
			if (m_editor)
				m_editor->updateStatusText(item->upload_limit,item->download_limit,false);
			
			if (item->set_conn_limits)
			{
				Out(SYS_SCD|LOG_NOTICE) << QString("Setting connection limits to : %1 per torrent, %2 global")
						.arg(item->torrent_conn_limit).arg(item->global_conn_limit) << endl;
				PeerManager::setMaxConnections(item->torrent_conn_limit);
				PeerManager::setMaxTotalConnections(item->global_conn_limit);
			}
			else
			{
				PeerManager::setMaxConnections(Settings::maxConnections());
				PeerManager::setMaxTotalConnections(Settings::maxTotalConnections());
			}
		}
		
		// now calculate the new interval
		int wait_time = m_schedule->getTimeToNextScheduleEvent(now) * 1000;
		if (wait_time < 1000)
			wait_time = 1000;
		m_timer.stop();
		m_timer.start(wait_time);
	}
	
	void BWSchedulerPlugin::onLoaded(Schedule* ns)
	{
		delete m_schedule;
		m_schedule = ns;
		m_editor->setSchedule(ns);
		timerTriggered();
	}
		
	
	bool BWSchedulerPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}
	
	void BWSchedulerPlugin::colorsChanged()
	{
		if (m_editor)
		{
			m_editor->setSchedule(m_schedule);
			m_editor->colorsChanged();
		}
	}
}
