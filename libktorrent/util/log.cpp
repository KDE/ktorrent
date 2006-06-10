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
#include <klocale.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qptrlist.h>
#include <iostream>
#include <interfaces/logmonitorinterface.h>
#include <qmutex.h> 
#include "log.h"
#include "error.h"

using namespace kt;

namespace bt
{
	class Log::Private
	{
	public:
		QTextStream* out;
		QFile fptr;
		bool to_cout;
		QPtrList<LogMonitorInterface> monitors;
		QString tmp;
		QMutex mutex;
	public:
		Private() : out(0),to_cout(false)
		{
			out = new QTextStream();
		}

		~Private()
		{
			delete out;
		}

		void setOutputFile(const QString & file)
		{
			if (fptr.isOpen())
				fptr.close();

			fptr.setName(file);
			if (!fptr.open(IO_WriteOnly))
				throw Error(i18n("Cannot open log file %1 : %2").arg(file).arg(fptr.errorString()));

			out->setDevice(&fptr);
		}

		void write(const QString & line)
		{
			mutex.lock();
			*out << line;
			if (to_cout)
				std::cout << line.local8Bit();

			tmp += line;
			mutex.unlock();
		}

		void endline()
		{
			mutex.lock();
			*out << ::endl;
			fptr.flush();
			if (to_cout)
				std::cout << std::endl;;
			
			if (monitors.count() > 0)
			{
				QPtrList<LogMonitorInterface>::iterator i = monitors.begin();
				while (i != monitors.end())
				{
					kt::LogMonitorInterface* lmi = *i;
					lmi->message(tmp);
					i++;
				}
			}
			tmp = "";
			mutex.unlock();
		}
	};
	
	Log::Log() 
	{
		priv = new Private();
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
}	
