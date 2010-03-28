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
#include "functions.h"

namespace bt
{

	Timer::Timer() : elapsed(0)
	{
		last = Now();
	}

	Timer::Timer(const Timer & t) : last(t.last),elapsed(t.elapsed)
	{}
			
	Timer::~Timer()
	{}


	TimeStamp Timer::update()
	{
		TimeStamp now = Now();
		TimeStamp d = (now > last) ? now - last : 0;
		elapsed = d;
		last = now;
		return last;
	}
	
	TimeStamp Timer::getElapsedSinceUpdate() const
	{
		TimeStamp now = Now();
		return (now > last) ? now - last : 0;
	}
	
	Timer & Timer::operator = (const Timer & t)
	{
		last = t.last;
		elapsed = t.elapsed;
		return *this;
	}
}
