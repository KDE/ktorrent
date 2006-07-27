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
#include "bwscheduler.h"
#include "schedulerpluginsettings.h"

#include <torrent/globals.h>
#include <torrent/queuemanager.h>

#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>

#include <net/socketmonitor.h>

#include <util/constants.h>
#include <util/log.h>

#include <qdatetime.h>
#include <qfile.h>
#include <qptrlist.h>

#include <kglobal.h>
#include <kstandarddirs.h>

using namespace bt;

namespace kt
{
	// ****** BWS *----------------------------------------------------------------------
	BWS::BWS()
	{
		m_schedule = new ScheduleCategory*[7];
		for(int i=0; i<7; ++i)
			m_schedule[i] = new ScheduleCategory[24];
		
		reset();
	}
	
	BWS& kt::BWS::operator=(const BWS& b)
	{
		for(int i=0; i<7; ++i)
			for(int j=0; j<24; ++j)
				m_schedule[i][j] = b.m_schedule[i][j];
		
		for(int i=0; i<3; ++i)
		{
			download[i] = b.download[i];
			upload[i] = b.upload[i];
		}
		
		return *this;
	}
	
	int BWS::getDownload(int cat)
	{
		return download[cat];
	}

	int BWS::getUpload(int cat)
	{
		return upload[cat];
	}

	void BWS::setDownload(int cat, int val)
	{
		download[cat] = val;
	}

	void BWS::setUpload(int cat, int val)
	{
		upload[cat] = val;
	}
	
	void BWS::setCategory(int day, int hour, ScheduleCategory val)
	{
		m_schedule[day][hour] = val;
	}

	ScheduleCategory BWS::getCategory(int day, int hour)
	{
		return m_schedule[day][hour];
	}
	
	void BWS::reset()
	{
		for(int i=0; i<7; ++i)
			for(int j=0; j<24; ++j)
				m_schedule[i][j] = CAT_NORMAL;
		
		for(int i=0; i<3; ++i)
		{
			download[i] = 0;
			upload[i] = 0;
		}
	}
	
	BWS::~BWS()
	{
		for(int i=0; i<7; ++i)
			delete [] m_schedule[i];
		
		delete [] m_schedule;
	}
	
	void BWS::debug()
	{
		for(int i=0; i<7; ++i)
		{
			Log & lg = Out();
			for(int j=0; j<24; ++j)
				lg << m_schedule[i][j];
			lg << endl;
		}
	}
	
	// ---- BWScheduler --------------------------------------------------------------------

	BWScheduler::BWScheduler()
		: m_core(0)
	{
		m_enabled = SchedulerPluginSettings::enableBWS();
		loadSchedule();
	}
	
	BWScheduler::~BWScheduler()
	{
	}
	
	void BWScheduler::setSchedule(const BWS& sch)
	{
		Out(SYS_SCD|LOG_NOTICE) << "BWS: Setting new schedule..." << endl;
		m_schedule = sch;
		saveSchedule();
		trigger();
	}
	
	void BWScheduler::setCoreInterface(CoreInterface* core)
	{
		m_core = core;
	}
	
	void BWScheduler::trigger()
	{
		if(!m_enabled)
			return;
		
		QDateTime now = QDateTime::currentDateTime();

		QString prefix = QString("BWS: %1 :: ").arg(now.toString());
		
		int t1 = now.date().dayOfWeek();
		int t2 = now.time().hour();
		ScheduleCategory sch = m_schedule.getCategory(t1-1, t2);
		
		switch(sch)
		{
			case CAT_NORMAL:
				Out(SYS_SCD|LOG_NOTICE) << prefix << "Switching to NORMAL category" << endl;
				Out(SYS_SCD|LOG_NOTICE) << prefix << QString("%1 Up, %2 Down")
						.arg(m_core->getMaxUploadSpeed()).arg(m_core->getMaxDownloadSpeed()) << endl;
				if(!m_core)
					break;
				m_core->setPausedState(false);
				net::SocketMonitor::setDownloadCap(1024 * m_core->getMaxDownloadSpeed());
				net::SocketMonitor::setUploadCap(1024 * m_core->getMaxUploadSpeed());
				break;
			case CAT_FIRST:
				Out(SYS_SCD|LOG_NOTICE) << prefix << "Switching to FIRST category" << endl;
				Out(SYS_SCD|LOG_NOTICE) << prefix << QString("%1 Up, %2 Down")
						.arg(m_schedule.getUpload(0)).arg(m_schedule.getDownload(0)) << endl;
				if(!m_core)
					break;
				m_core->setPausedState(false);
				net::SocketMonitor::setDownloadCap(1024 * m_schedule.getDownload(0));
				net::SocketMonitor::setUploadCap(1024 * m_schedule.getUpload(0));
				break;
			case CAT_SECOND:
				Out(SYS_SCD|LOG_NOTICE) << prefix << "Switching to SECOND category" << endl;
				Out(SYS_SCD|LOG_NOTICE) << prefix << QString("%1 Up, %2 Down")
						.arg(m_schedule.getUpload(1)).arg(m_schedule.getDownload(1)) << endl;
				if(!m_core)
					break;
				m_core->setPausedState(false);
				net::SocketMonitor::setDownloadCap(1024 * m_schedule.getDownload(1));
				net::SocketMonitor::setUploadCap(1024 * m_schedule.getUpload(1));
				break;
			case CAT_THIRD:
				Out(SYS_SCD|LOG_NOTICE) << prefix << "Switching to THIRD category" << endl;
				Out(SYS_SCD|LOG_NOTICE) << prefix << QString("%1 Up, %2 Down")
						.arg(m_schedule.getUpload(2)).arg(m_schedule.getDownload(2)) << endl;
				if(!m_core)
					break;
				m_core->setPausedState(false);
				net::SocketMonitor::setDownloadCap(1024 * m_schedule.getDownload(2));
				net::SocketMonitor::setUploadCap(1024 * m_schedule.getUpload(2));
				break;
			case CAT_OFF:
				Out(SYS_SCD|LOG_NOTICE) << prefix << "Switching to OFF" << endl;

				if(!m_core)
					break;
				m_core->setPausedState(true);
				break;
		}
	}
	
	void BWScheduler::loadSchedule()
	{
		QFile file(KGlobal::dirs()->saveLocation("data","ktorrent") + "bwschedule");

		if(!file.exists())
			return;

		file.open(IO_ReadOnly);
		QDataStream stream(&file);

		int tmp;

		//extract category values
		for(int i=0; i<3; ++i)
		{
			stream >> tmp;
			m_schedule.setDownload(i, tmp);
			stream >> tmp;
			m_schedule.setUpload(i, tmp);
		}

		//extract schedule
		for(int i=0; i<7; ++i) 
		{
			for(int j=0; j<24; ++j) 
			{
				stream >> tmp;
				m_schedule.setCategory(i, j, (ScheduleCategory) tmp);
			}
		}

		file.close();
	}

	void BWScheduler::saveSchedule()
	{
		QFile file(KGlobal::dirs()->saveLocation("data","ktorrent") + "bwschedule");

		file.open(IO_WriteOnly);
		QDataStream stream(&file);

		for(int i=0; i<3; ++i)
		{
			stream << m_schedule.getDownload(i);
			stream << m_schedule.getUpload(i);
		}

		//Now schedule
		for(int i=0; i<7; ++i)
			for(int j=0; j<24; ++j)
				stream << (int) m_schedule.getCategory(i, j);

		file.close();
	}
	
	void BWScheduler::setEnabled(bool theValue)
	{
		m_enabled = theValue;
	}
}
