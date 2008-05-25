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

#include <kurl.h>
#include <kprocess.h>
#include <klocale.h>
#include <qdatetime.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qptrlist.h>
#include <iostream>
#include <stdlib.h>
#include <torrent/globals.h>
#include <interfaces/logmonitorinterface.h>
#include <qmutex.h> 
#include <util/fileops.h>
#include <stdlib.h>
#include "log.h"
#include "error.h"
#include "autorotatelogjob.h"

using namespace kt;

namespace bt
{
	const Uint32 MAX_LOG_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
	
	class Log::Private
	{
	public:
		Log* parent;
		QTextStream* out;
		QFile fptr;
		bool to_cout;
		QPtrList<LogMonitorInterface> monitors;
		QString tmp;
		QMutex mutex;
		unsigned int m_filter;
		AutoRotateLogJob* rotate_job;
	public:
		Private(Log* parent) : parent(parent),out(0),to_cout(false),rotate_job(0)
		{
			out = new QTextStream();
		}

		~Private()
		{
			delete out;
		}

		
		void setFilter(unsigned int filter)
		{
			m_filter = filter;
		}
		
		void rotateLogs(const QString & file)
		{
			if (bt::Exists(file + "-10.gz"))
				bt::Delete(file + "-10.gz",true);
			
			// move all log files one up
			for (Uint32 i = 10;i > 1;i--)
			{
				QString prev = QString("%1-%2.gz").arg(file).arg(i - 1);
				QString curr = QString("%1-%2.gz").arg(file).arg(i);
				if (bt::Exists(prev))
					bt::Move(prev,curr,true);
			}
			
			// move current log to 1 and zip it
			bt::Move(file,file + "-1",true);
			system(QString("gzip " + KProcess::quote(file + "-1")).local8Bit());
		}

		void setOutputFile(const QString & file)
		{
			if (fptr.isOpen())
				fptr.close();
			
			if (bt::Exists(file))
				rotateLogs(file);

			fptr.setName(file);
			if (!fptr.open(IO_WriteOnly))
				throw Error(i18n("Cannot open log file %1 : %2").arg(file).arg(fptr.errorString()));

			out->setDevice(&fptr);
		}

		void write(const QString & line)
		{
			tmp += line;
		}
		
		void finishLine()
		{
			// only add stuff when we are not rotating the logs
			// this could result in the loss of some messages
			if (!rotate_job) 
			{
				*out << QDateTime::currentDateTime().toString() << ": " << tmp << ::endl;
				fptr.flush();
				if (to_cout)
					std::cout << tmp.local8Bit() << std::endl;
				
				if (monitors.count() > 0)
				{
					QPtrList<LogMonitorInterface>::iterator i = monitors.begin();
					while (i != monitors.end())
					{
						kt::LogMonitorInterface* lmi = *i;
						lmi->message(tmp,m_filter);
						i++;
					}
				}
			}
			tmp = "";
		}

		void endline()
		{
			finishLine();
			if (fptr.size() > MAX_LOG_FILE_SIZE && !rotate_job)
			{
				tmp = "Log larger then 10 MB, rotating";
				finishLine();
				QString file = fptr.name();
				fptr.close(); // close the log file
				out->setDevice(0);		
				// start the rotate job
				rotate_job = new AutoRotateLogJob(file,parent); 
			}
		}
		
		void logRotateDone()
		{
			fptr.open(IO_WriteOnly);
			out->setDevice(&fptr);
			rotate_job = 0;
		}
	};
	
	Log::Log() 
	{
		priv = new Private(this);
	}
	
	
	Log::~Log()
	{
		delete priv;
	}
	
	
	void Log::setOutputFile(const QString & file)
	{
		priv->setOutputFile(file);
	}

	void Log::addMonitor(kt::LogMonitorInterface* m)
	{
		priv->monitors.append(m);
	}

	void Log::removeMonitor(kt::LogMonitorInterface* m)
	{
		priv->monitors.remove(m);
	}

	void Log::setOutputToConsole(bool on)
	{
		priv->to_cout = on;
	}

	Log & endl(Log & lg)
	{
		lg.priv->endline();
		lg.priv->mutex.unlock(); // unlock after end of line
		return lg;
	}

	Log & Log::operator << (const KURL & url)
	{
		priv->write(url.prettyURL());
		return *this;
	}

	Log & Log::operator << (const QString & s)
	{
		priv->write(s);
		return *this;
	}

	Log & Log::operator << (const char* s)
	{
		priv->write(s);
		return *this;
	}

	Log & Log::operator << (Uint64 v)
	{
		return operator << (QString::number(v));
	}

	Log & Log::operator << (Int64 v)
	{
		return operator << (QString::number(v));
	}

	void Log::setFilter(unsigned int filter)
	{
		priv->setFilter(filter);
	}

	void Log::lock()
	{
		priv->mutex.lock();
	}
	
	void Log::logRotateDone()
	{
		priv->logRotateDone();
	}
	
	Log & Out(unsigned int arg)
	{
		Log & lg = Globals::instance().getLog(arg);
		lg.lock();
		return lg;
	}
}
