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

		return true;
	}

	void LogFlags::updateFlags()
	{
		m_flags.SYSGEN = LogViewerPluginSettings::sysGEN();
		m_flags.SYSCON = LogViewerPluginSettings::sysCON();
		m_flags.SYSDHT = LogViewerPluginSettings::sysDHT();
		m_flags.SYSTRK = LogViewerPluginSettings::sysTRK();
		m_flags.SYSDIO = LogViewerPluginSettings::sysDIO();

		m_flags.SYSINW = LogViewerPluginSettings::sysINW();
		m_flags.SYSIPF = LogViewerPluginSettings::sysIPF();
		m_flags.SYSPFI = LogViewerPluginSettings::sysPFI();
		m_flags.SYSPNP = LogViewerPluginSettings::sysPNP();
		m_flags.SYSSCD = LogViewerPluginSettings::sysSCD();
		m_flags.SYSSNF = LogViewerPluginSettings::sysSNF();
		m_flags.SYSSRC = LogViewerPluginSettings::sysSRC();
		m_flags.SYSRSS = LogViewerPluginSettings::sysRSS();
		m_flags.SYSWEB = LogViewerPluginSettings::sysWEB();
		
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
