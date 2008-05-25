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

#include "timer.h"

namespace bt
{

	Timer::Timer() : elapsed(0)
	{
		last = QTime::currentTime();
	}

	Timer::Timer(const Timer & t) : last(t.last),elapsed(t.elapsed)
	{}
			
	Timer::~Timer()
	{}


	void Timer::update()
	{
		QTime now = QTime::currentTime();

		int d = last.msecsTo(now);
		if (d < 0)
			d = 0;
		elapsed = d;
		last = now;
	}
	
	Uint32 Timer::getElapsedSinceUpdate() const
	{
		QTime now = QTime::currentTime();
		int d = last.msecsTo(now);
		if (d < 0)
			d = 0;
		return d;
	}
	
	Timer & Timer::operator = (const Timer & t)
	{
		last = t.last;
		elapsed = t.elapsed;
		return *this;
	}
}
