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
#include <qpair.h>
#include <qvaluelist.h>
#include <util/log.h>
#include <util/timer.h>
#include "speedestimater.h"
#include <util/functions.h>

namespace bt
{
	class SpeedEstimater::SpeedEstimaterPriv
	{
		float rate;
		QValueList<QPair<Uint32,TimeStamp> > dlrate;
	public:
		SpeedEstimaterPriv() : rate(0) {}
		~SpeedEstimaterPriv() {}
		
		void data(Uint32 bytes)
		{
			dlrate.append(qMakePair(bytes,GetCurrentTime()));
		}
		
		void update()
		{
			TimeStamp now = GetCurrentTime();
			
			Uint32 bytes = 0,oldest = now;
			QValueList<QPair<Uint32,TimeStamp> >::iterator i = dlrate.begin();
			while (i != dlrate.end())
			{
				QPair<Uint32,TimeStamp> & p = *i;
				if (now - p.second > 3000)
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
			
			Uint32 d = 3000;
			
			if (bytes == 0)
			{
				rate = 0;
			}
			else
			{
			//	Out() << "bytes = " << bytes << " d = " << d << endl;
				rate = (float) bytes / (d * 0.001f);
			}
		}
		
		float getRate() const {return rate;}
	};

	SpeedEstimater::SpeedEstimater()
	{
		download_rate = 0;
		down = new SpeedEstimaterPriv();
	}


	SpeedEstimater::~SpeedEstimater()
	{
		delete down;
	}

	
	
	void SpeedEstimater::onRead(Uint32 bytes)
	{
		down->data(bytes);
	}
	
	void SpeedEstimater::update()
	{
		down->update();
		download_rate = down->getRate();
	}
}
