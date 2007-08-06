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
			return (arg & m_flags.SYSCON) && ((arg & 0x0000000F) <= m_flags.SYSCON);

		if(arg & SYS_DHT)
			return (arg & m_flags.SYSDHT) && ((arg & 0x0000000F) <= m_flags.SYSDHT);

		if(arg & SYS_TRK)
			return (arg & m_flags.SYSTRK) && ((arg & 0x0000000F) <= m_flags.SYSTRK);
		
		if(arg & SYS_DIO)
			return (arg & m_flags.SYSDIO) && ((arg & 0x0000000F) <= m_flags.SYSDIO);

		if(arg & SYS_INW)
			return (arg & m_flags.SYSINW) && ((arg & 0x0000000F) <= m_flags.SYSINW);

		if(arg & SYS_IPF)
			return (arg & m_flags.SYSIPF) && ((arg & 0x0000000F) <= m_flags.SYSIPF);

		if(arg & SYS_PFI)
			return (arg & m_flags.SYSPFI) && ((arg & 0x0000000F) <= m_flags.SYSPFI);

		if(arg & SYS_PNP)
			return (arg & m_flags.SYSPNP) && ((arg & 0x0000000F) <= m_flags.SYSPNP);

		if(arg & SYS_SCD)
			return (arg & m_flags.SYSSCD) && ((arg & 0x0000000F) <= m_flags.SYSSCD);

		if(arg & SYS_SNF)
			return (arg & m_flags.SYSSNF) && ((arg & 0x0000000F) <= m_flags.SYSSNF);

		if(arg & SYS_SRC)
			return (arg & m_flags.SYSSRC) && ((arg & 0x0000000F) <= m_flags.SYSSRC);
		
		if(arg & SYS_RSS)
			return (arg & m_flags.SYSRSS) && ((arg & 0x0000000F) <= m_flags.SYSRSS);
		
		if(arg & SYS_WEB)
			return (arg & m_flags.SYSWEB) && ((arg & 0x0000000F) <= m_flags.SYSWEB);

		if (arg & SYS_ZCO)
			return (arg & m_flags.SYSZCO) && ((arg & 0x0000000F) <= m_flags.SYSZCO);

		return true;
	}

	void LogFlags::updateFlags()
	{
		// Possible values in config : (due to how KConfigDialog works, so we need to convert this)
		// 0 = All
		// 1 = Debug
		// 2 = Notice
		// 3 = Important
		// 4 = None
		unsigned int flags[] = {LOG_ALL,LOG_DEBUG,LOG_NOTICE,LOG_IMPORTANT,LOG_NONE};

		m_flags.SYSGEN = flags[LogViewerPluginSettings::sysGEN()];
		m_flags.SYSCON = flags[LogViewerPluginSettings::sysCON()];
		m_flags.SYSDHT = flags[LogViewerPluginSettings::sysDHT()];
		m_flags.SYSTRK = flags[LogViewerPluginSettings::sysTRK()];
		m_flags.SYSDIO = flags[LogViewerPluginSettings::sysDIO()];

		m_flags.SYSINW = flags[LogViewerPluginSettings::sysINW()];
		m_flags.SYSIPF = flags[LogViewerPluginSettings::sysIPF()];
		m_flags.SYSPFI = flags[LogViewerPluginSettings::sysPFI()];
		m_flags.SYSPNP = flags[LogViewerPluginSettings::sysPNP()];
		m_flags.SYSSCD = flags[LogViewerPluginSettings::sysSCD()];
		m_flags.SYSSNF = flags[LogViewerPluginSettings::sysSNF()];
		m_flags.SYSSRC = flags[LogViewerPluginSettings::sysSRC()];
		m_flags.SYSRSS = flags[LogViewerPluginSettings::sysRSS()];
		m_flags.SYSWEB = flags[LogViewerPluginSettings::sysWEB()];
		m_flags.SYSZCO = flags[LogViewerPluginSettings::sysZCO()];
		
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
		
		if(arg & 0x04) // Debug
		{
			line.prepend("<font color=\"#646464\">");
			line.append("</font>");
			return line;
		}
		
		if(arg & 0x02) // Notice 
		{
			line.prepend("<font color=\"#000000\">");
			line.append("</font>");
			return line; 
		}
		
		if(arg & 0x01) // Important
		{
			line.prepend("<b>");
			line.append("</b>");
			return line;
		}
		
		return line;
	}
}
