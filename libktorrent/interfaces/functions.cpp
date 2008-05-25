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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <qdatetime.h>
#include <klocale.h>
#include <kglobal.h>
#include "functions.h"

using namespace bt;

namespace kt
{
	
	
	QString BytesToString(Uint64 bytes,int precision)
	{
		KLocale* loc = KGlobal::locale();
		if (bytes >= 1024 * 1024 * 1024)
			return i18n("%1 GB").arg(loc->formatNumber(bytes / TO_GIG,precision < 0 ? 2 : precision));
		else if (bytes >= 1024*1024)
			return i18n("%1 MB").arg(loc->formatNumber(bytes / TO_MEG,precision < 0 ? 1 : precision));
		else if (bytes >= 1024)
			return i18n("%1 KB").arg(loc->formatNumber(bytes / TO_KB,precision < 0 ? 1 : precision));
		else
			return i18n("%1 B").arg(bytes);
	}

	QString KBytesPerSecToString(double speed,int precision)
	{
		KLocale* loc = KGlobal::locale();
		return i18n("%1 KB/s").arg(loc->formatNumber(speed,precision));
	}

	QString DurationToString(Uint32 nsecs)
	{
		KLocale* loc = KGlobal::locale();
		QTime t;
		int ndays = nsecs / 86400;
		t = t.addSecs(nsecs % 86400);
		QString s = loc->formatTime(t,true,true);
		if (ndays > 0)
			s = i18n("1 day ","%n days ",ndays) + s;

		return s;
	}
}
