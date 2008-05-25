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
#if 0
#include <math.h>
#include "cap.h"

namespace bt
{
	typedef QValueList<Cap::Entry>::iterator CapItr;

	Cap::Cap(bool percentage_check) : max_bytes_per_sec(0),leftover(0),current_speed(0),percentage_check(percentage_check)
	{
		timer.update();
	}


	Cap::~Cap()
	{}

	void Cap::setMaxSpeed(Uint32 max)
	{
		max_bytes_per_sec = max;
		// tell everybody to go wild
		if (max_bytes_per_sec == 0)
		{
			CapItr i = entries.begin();
			while (i != entries.end())
			{
				Cap::Entry & e = *i;
				e.obj->proceed(0);
				i++;
			}
			entries.clear();
			leftover = 0;
		}
	}
	
	bool Cap::allow(Cappable* pd,Uint32 bytes)
	{
		if (max_bytes_per_sec == 0 || (percentage_check && (double)current_speed / (double)max_bytes_per_sec < 0.75))
		{
			timer.update();
			return true;
		}

		// append pd to queue
		entries.append(Cap::Entry(pd,bytes));
		return false;
	}

	void Cap::killed(Cappable*  pd)
	{
		CapItr i = entries.begin();
		while (i != entries.end())
		{
			Cap::Entry & e = *i;
			if (e.obj == pd)
				i = entries.erase(i);
			else
				i++;
		}
	}

	void Cap::update()
	{
		if (entries.count() == 0)
		{
			timer.update();
			return;
		}
		
		// first calculate the time since the last update
		double el = timer.getElapsedSinceUpdate();
		
		// calculate the number of bytes we can send, including those leftover from the last time
		Uint32 nb = (Uint32)round((el / 1000.0) * max_bytes_per_sec) + leftover;
		leftover = 0;
	//	Out() << "nb = " << nb << endl;
		
		while (entries.count() > 0 && nb > 0)
		{
			// get the first
			Cap::Entry & e = entries.first();
			
			if (e.num_bytes <= nb)
			{
				nb -= e.num_bytes;
				// we can send all remaining bytes of the packet
				e.obj->proceed(e.num_bytes);
				entries.pop_front();
			}
			else
			{
				// sent nb bytes of the packets
				e.obj->proceed(nb);
				e.num_bytes -= nb;
				nb = 0;
			}
		}
		
		leftover = nb;
		timer.update();
	}

}
#endif
