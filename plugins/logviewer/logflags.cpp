/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić   								   *
 *   ivasic@gmail.com   												   *
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
#include "logflags.h"
#include "logviewer.h"
#include "logviewerpluginsettings.h"

#include <util/log.h>
#include <torrent/globals.h>

#include <qstring.h>

using namespace bt;

namespace kt
{

	LogFlags* LogFlags::self = 0;
	LogViewer* LogFlags::m_log = 0;

	LogFlags::LogFlags()
	{
		updateFlags();
	}

	LogFlags::~LogFlags()
	{}

	LogFlags& LogFlags::instance()
	{
		if (!self)
			self = new LogFlags();
		return *self;
	}

	bool LogFlags::checkFlags(unsigned int arg)
	{
		if(arg & SYS_GEN)
			return m_flags.SYSGEN & arg;

		if(arg & SYS_CON)
			return m_flags.SYSCON & arg;

		if(arg & SYS_DHT)
			return m_flags.SYSDHT & arg;

		if(arg & SYS_TRK)
			return m_flags.SYSTRK & arg;

		if(arg & SYS_INW)
			return m_flags.SYSINW & arg;

		if(arg & SYS_IPF)
			return m_flags.SYSIPF & arg;

		if(arg & SYS_PFI)
			return m_flags.SYSPFI & arg;

		if(arg & SYS_PNP)
			return m_flags.SYSPNP & arg;

		if(arg & SYS_SCD)
			return m_flags.SYSSCD & arg;

		if(arg & SYS_SNF)
			return m_flags.SYSSNF & arg;

		if(arg & SYS_SRC)
			return m_flags.SYSSRC & arg;

		return true;
	}

	void LogFlags::updateFlags()
	{
		m_flags.SYSGEN = LogViewerPluginSettings::sysGEN();
		m_flags.SYSCON = LogViewerPluginSettings::sysCON();
		m_flags.SYSDHT = LogViewerPluginSettings::sysDHT();
		m_flags.SYSTRK = LogViewerPluginSettings::sysTRK();

		m_flags.SYSINW = LogViewerPluginSettings::sysINW();
		m_flags.SYSIPF = LogViewerPluginSettings::sysIPF();
		m_flags.SYSPFI = LogViewerPluginSettings::sysPFI();
		m_flags.SYSPNP = LogViewerPluginSettings::sysPNP();
		m_flags.SYSSCD = LogViewerPluginSettings::sysSCD();
		m_flags.SYSSNF = LogViewerPluginSettings::sysSNF();
		m_flags.SYSSRC = LogViewerPluginSettings::sysSRC();
		
		m_useRichText = LogViewerPluginSettings::useRichText();
		
		if(m_log)
			m_log->setRichText(m_useRichText);
	}
	
	void LogFlags::finalize()
	{
		delete self;
		self = 0;
		m_log = 0;
	}
	
	bool LogFlags::useRichText()
	{
		return m_useRichText;
	}
	
	void LogFlags::setLog(LogViewer* log)
	{
		m_log = log;
	}
	
	QString& LogFlags::getFormattedMessage(unsigned int arg, QString& line)
	{
		if( (arg & LOG_ALL) == LOG_ALL)
		{
			return line;
		}
		
		if(arg & 0x04)
		{
			line.prepend("<font color=\"#646464\">");
			line.append("</font>");
			return line;
		}
		
		if(arg & 0x02)
		{
			line.prepend("<font color=\"#000000\">");
			line.append("</font>");
			return line;
		}
		
		if(arg & 0x01)
		{
			line.prepend("<font color=\"#460000\">");
			line.append("</font>");
			return line;
		}
		
		return line;
	}
}
