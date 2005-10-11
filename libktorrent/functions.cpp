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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <klocale.h>
#include <kglobal.h>
#include "functions.h"

using namespace bt;

namespace kt
{
	const double TO_KB = 1024.0;
	const double TO_MEG = (1024.0 * 1024.0);
	const double TO_GIG = (1024.0 * 1024.0 * 1024.0);

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
		return i18n("%1 KB/sec").arg(loc->formatNumber(speed,precision));
	}
}
