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
#include <sys/time.h> 
#include "timer.h"

namespace bt
{

	Timer::Timer()
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		elapsed = 0;
		last = tv.tv_sec * 1000 + tv.tv_usec * 0.001;
	}

	Timer::Timer(const Timer & t) : last(t.last),elapsed(t.elapsed)
	{}
			
	Timer::~Timer()
	{}


	void Timer::update()
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		double n = tv.tv_sec * 1000 + tv.tv_usec * 0.001;
		elapsed = n - last;
		last = n;
	}
	
	double Timer::getElapsedSinceUpdate() const
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		double n = tv.tv_sec * 1000 + tv.tv_usec * 0.001;
		return n - last;
	}
	
	Timer & Timer::operator = (const Timer & t)
	{
		last = t.last;
		elapsed = t.elapsed;
		return *this;
	}
}
