/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <kglobal.h>
#include <kconfig.h>
#include "logviewer.h"
#include "logflags.h"
#include "logviewerpluginsettings.h"

namespace kt
{

	LogViewer::LogViewer(QWidget *parent, const char *name)
			: KTextBrowser(parent, name), LogMonitorInterface()
	{
		if(m_useRichText = LogViewerPluginSettings::useRichText())
			setTextFormat(Qt::RichText);
		else
			setTextFormat(Qt::PlainText);
		
		setMaxLogLines(100);
		setMinimumSize(QSize(0,50));
		setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Minimum);
		KGlobal::config()->setGroup("LogViewer");
		if (KGlobal::config()->hasKey("LogViewerWidgetSize"))
		{
			QSize s = KGlobal::config()->readSizeEntry("LogViewerWidgetSize",0);
			resize(s);
		}
		
		LogFlags::instance().setLog(this);
	}


	LogViewer::~LogViewer()
	{
		KGlobal::config()->setGroup("LogViewer");
		KGlobal::config()->writeEntry("LogViewerWidgetSize",size());
		LogFlags::instance().setLog(0);
	}


	void LogViewer::message(const QString& line, unsigned int arg)
	{
		if(arg==0x00 || LogFlags::instance().checkFlags(arg))
		{
			if(m_useRichText)
			{
				QString tmp = line;
				append(LogFlags::instance().getFormattedMessage(arg, tmp));
			}
			else
				append(line);
		}
	}
	
	void LogViewer::setRichText(bool val)
	{
		if(val)
			setTextFormat(Qt::RichText);
		else
			setTextFormat(Qt::PlainText);
		
		m_useRichText = val;
	}
}
#include "logviewer.moc"
