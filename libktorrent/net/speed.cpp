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
#include <util/log.h>
#include <util/timer.h>
#include <util/functions.h>
#include "speed.h"

using namespace bt;

namespace net
{
	const Uint64 SPEED_INTERVAL = 5000;

	Speed::Speed() : rate(0),bytes(0)
	{}


	Speed::~Speed()
	{}
	
	void Speed::onData(Uint32 b,bt::TimeStamp ts)
	{
		dlrate.append(qMakePair(b,ts));
		bytes += b;
	}

	void Speed::update(bt::TimeStamp now)
	{	
		QValueList<QPair<Uint32,TimeStamp> >::iterator i = dlrate.begin();
		while (i != dlrate.end())
		{
			QPair<Uint32,TimeStamp> & p = *i;
			if (now - p.second > SPEED_INTERVAL || now < p.second)
			{
				if (bytes >= p.first) // make sure we don't wrap around
					bytes -= p.first; // subtract bytes
				else
					bytes = 0;
				i = dlrate.erase(i);
			}
			else
			{	
				// seeing that newer entries are appended, they are in the list chronologically
				// so once we hit an entry which is in the interval, we can just break out of the loop
				// because all following entries will be in the interval
				break;
			}
		}
			
		if (bytes == 0)
		{
			rate = 0;
		}
		else
		{
			//	Out() << "bytes = " << bytes << " d = " << d << endl;
			rate = (float) bytes / (float)(SPEED_INTERVAL * 0.001);
		}
	}

}
