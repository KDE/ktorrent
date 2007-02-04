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
#include <qapplication.h>
#include "logviewer.h"
#include "logflags.h"
#include "logviewerpluginsettings.h"

namespace kt
{
	const int LOG_EVENT_TYPE = 65432;
	
	class LogEvent : public QCustomEvent
	{
		QString str;
	public:
		LogEvent(const QString & str) : QCustomEvent(LOG_EVENT_TYPE),str(str)
		{}
		
		virtual ~LogEvent()
		{}
		
		const QString & msg() const {return str;}
	};

	LogViewer::LogViewer(QWidget *parent, const char *name)
			: KTextBrowser(parent, name), LogMonitorInterface()
	{
		/*
		IMPORTANT: use LogText mode, so that setMaxLogLines will work, if not everything will be kept in memory.
		*/
		setTextFormat(Qt::LogText);
		setMaxLogLines(200);
		setMinimumSize(QSize(0,50));
		setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
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
		/*
			IMPORTANT: because QTextBrowser is not thread safe, we must use the Qt event mechanism 
			to add strings to it, this will ensure that strings will only be added in the main application
			thread.
		*/
		if(arg==0x00 || LogFlags::instance().checkFlags(arg))
		{
			if(m_useRichText)
			{
				QString tmp = line;
				LogEvent* le = new LogEvent(LogFlags::instance().getFormattedMessage(arg, tmp));
				QApplication::postEvent(this,le);
			}
			else
			{
				LogEvent* le = new LogEvent(line);
				QApplication::postEvent(this,le);
			}
		}
	}
	
	void LogViewer::customEvent(QCustomEvent* ev)
	{
		if (ev->type() == LOG_EVENT_TYPE)
		{
			LogEvent* le = (LogEvent*)ev;
			append(le->msg());
		}
	}
	
	void LogViewer::setRichText(bool val)
	{
		m_useRichText = val;
	}
}
#include "logviewer.moc"
