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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <math.h>
#include <util/functions.h>
#include "upspeedestimater.h"

namespace bt
{

	UpSpeedEstimater::UpSpeedEstimater()
	{
		accumulated_bytes = 0;
		upload_rate = 0.0;
	}


	UpSpeedEstimater::~UpSpeedEstimater()
	{}


	void UpSpeedEstimater::writeBytes(Uint32 bytes,bool rec)
	{
		// add entry to outstanding_bytes
		Entry e;
		e.bytes = bytes;
		e.data = rec;
		e.start_time = GetCurrentTime();
		outstanding_bytes.append(e);
	}
		
	void UpSpeedEstimater::bytesWritten(Uint32 bytes)
	{
		QValueList<Entry>::iterator i = outstanding_bytes.begin();
		Uint32 now = GetCurrentTime();
		while (bytes > 0 && i != outstanding_bytes.end())
		{
			Entry e = *i;
			if (e.bytes <= bytes + accumulated_bytes)
			{
				// first remove outstanding bytes
				i = outstanding_bytes.erase(i);
				bytes -= e.bytes;
				accumulated_bytes = 0;
				if (e.data)
				{
					// if it's data move it to the written_bytes list
					// but first store time it takes to send in e.t
					e.duration = now - e.start_time;
					written_bytes.append(e);
				}
			}
			else
			{
				accumulated_bytes += bytes;
				bytes = 0;
			}
		}
	}

	void UpSpeedEstimater::update()
	{
		upload_rate = 0;
		if (written_bytes.empty())
			return;

		Uint32 now = GetCurrentTime();
		const Uint32 INTERVAL = 3000;
		
		Uint32 tot_bytes = 0;
		
		Uint32 oldest_time = now;
		QValueList<Entry>::iterator i = written_bytes.begin();
		while (i != written_bytes.end())
		{
			Entry & e = *i;
			Uint32 end_time = e.start_time + e.duration;
			
			if (now - end_time > INTERVAL)
			{
				// get rid of old entries
				i = written_bytes.erase(i);
			}
			else if (now - e.start_time <= INTERVAL)
			{
				// entry was fully sent in the last 3 seconds
				// so fully add it
				tot_bytes += e.bytes;
				if (e.start_time < oldest_time)
					oldest_time = e.start_time;
				i++;
			}
			else
			{
				// entry was partially sent in the last 3 seconds
				// so we need to take into account a part of the bytes;
				Uint32 part_dur = end_time - (now - INTERVAL);
				double dur_perc = (double)part_dur / e.duration;
				tot_bytes += (Uint32)ceil(dur_perc * e.bytes);
				oldest_time = (now - INTERVAL);
				i++;
			}
		}

		/*Uint32 tot_time = now - oldest_time;
		if (tot_time == 0)
			upload_rate = 0;
		else*/
			upload_rate = (double)tot_bytes / (INTERVAL * 0.001);
	}

}
