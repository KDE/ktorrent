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
#include <qpair.h>
#include <qvaluelist.h>
#include "log.h"
#include "timer.h"
#include "speedestimater.h"

namespace bt
{
	class SpeedEstimater::SpeedEstimaterPriv
	{
		float rate;
		QValueList<QPair<Uint32,Uint32> > dlrate;
	public:
		SpeedEstimaterPriv() : rate(0) {}
		~SpeedEstimaterPriv() {}
		
		void data(Uint32 bytes)
		{
			dlrate.append(qMakePair(bytes,GetCurrentTime()));
		}
		
		void update()
		{
			Uint32 now = GetCurrentTime();
			
			Uint32 bytes = 0,oldest = now;
			QValueList<QPair<Uint32,Uint32> >::iterator i = dlrate.begin();
			while (i != dlrate.end())
			{
				QPair<Uint32,Uint32> & p = *i;
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
		upload_rate = download_rate = 0;
		up = new SpeedEstimaterPriv();
		down = new SpeedEstimaterPriv();
		
	}


	SpeedEstimater::~SpeedEstimater()
	{
		delete up;
		delete down;
	}

	
	
	void SpeedEstimater::onWrite(Uint32 nbytes)
	{
		up->data(nbytes);
	}
	
	void SpeedEstimater::onRead(Uint32 bytes)
	{
		if (bytes > 4000000)
			Out() << "DODO" << endl;
		down->data(bytes);
	}
	
	void SpeedEstimater::update()
	{
		up->update();
		down->update();
		upload_rate = up->getRate();
		download_rate = down->getRate();
	}
}
