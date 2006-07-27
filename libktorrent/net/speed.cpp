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
	const Uint32 SPEED_INTERVAL = 2000;

	Speed::Speed()
	{}


	Speed::~Speed()
	{}
	
	void Speed::onData(Uint32 bytes)
	{
		dlrate.append(qMakePair(bytes,GetCurrentTime()));
	}

	void Speed::update()
	{
		Uint32 now = GetCurrentTime();
			
		Uint32 bytes = 0,oldest = now;
		QValueList<QPair<Uint32,Uint32> >::iterator i = dlrate.begin();
		while (i != dlrate.end())
		{
			QPair<Uint32,Uint32> & p = *i;
			if (now - p.second > SPEED_INTERVAL)
			{
				i = dlrate.erase(i);
			}
			else
			{
				if (p.second < oldest)
					oldest = p.second;
					
				bytes += p.first;
				i++;
			}
		}
			
		Uint32 d = SPEED_INTERVAL;
			
		if (bytes == 0)
		{
			rate = 0;
		}
		else
		{
			//	Out() << "bytes = " << bytes << " d = " << d << endl;
			rate = (float) bytes / (float)(d * 0.001);
		}
	}

}
