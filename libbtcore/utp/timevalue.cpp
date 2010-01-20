/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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

#include "timevalue.h"
#include <sys/time.h>

namespace utp
{
	
	TimeValue::TimeValue()
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		seconds = tv.tv_sec;
		microseconds = tv.tv_usec;
	}
	
	TimeValue::TimeValue(bt::Uint64 secs,bt::Uint64 usecs) : seconds(secs),microseconds(usecs)
	{
	}

	TimeValue::TimeValue(const utp::TimeValue& tv) : seconds(tv.seconds),microseconds(tv.microseconds)
	{
	}


	TimeValue& TimeValue::operator=(const utp::TimeValue& tv)
	{
		seconds = tv.seconds;
		microseconds = tv.microseconds;
		return *this;
	}

	bt::Int64 operator - (const utp::TimeValue& a, const utp::TimeValue& b)
	{
		bt::Int64 seconds = b.seconds - a.seconds;
		bt::Int64 microseconds = b.microseconds - a.microseconds;
		
		while (microseconds < 0)
		{
			microseconds += 1000000;
			seconds -= 1;
		}
		
		return (1000000LL * seconds + microseconds) / 1000;
	}

}

