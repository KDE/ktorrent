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
#include <math.h>
#include <util/functions.h>
#include <util/log.h>
#include <torrent/globals.h>
#include "upspeedestimater.h"

namespace bt
{

	UpSpeedEstimater::UpSpeedEstimater()
	{
		accumulated_bytes = 0;
		upload_rate = 0.0;
		proto_upload_rate = 0.0;
	}


	UpSpeedEstimater::~UpSpeedEstimater()
	{}


	void UpSpeedEstimater::writeBytes(Uint32 bytes,bool proto)
	{
		// add entry to outstanding_bytes
		Entry e;
		e.bytes = bytes;
		e.data = !proto;
		e.start_time = GetCurrentTime();
		outstanding_bytes.append(e);
	}
		
	void UpSpeedEstimater::bytesWritten(Uint32 bytes)
	{
		QValueList<Entry>::iterator i = outstanding_bytes.begin();
		TimeStamp now = GetCurrentTime();
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
				else
				{
					e.duration = now - e.start_time;
#ifdef MEASURE_PROTO_OVERHEAD
					proto_bytes.append(e);
#endif
				}
			}
			else
			{
				accumulated_bytes += bytes;
				bytes = 0;
			}
		}
	}
	
	double UpSpeedEstimater::rate(QValueList<Entry> & el)
	{
		TimeStamp now = GetCurrentTime();
		const Uint32 INTERVAL = 3000;
		
		Uint32 tot_bytes = 0;
		Uint32 oldest_time = now;
		
		QValueList<Entry>::iterator i = el.begin();
		while (i != el.end())
		{
			Entry & e = *i;
			Uint32 end_time = e.start_time + e.duration;
			
			if (now - end_time > INTERVAL)
			{
				// get rid of old entries
				i = el.erase(i);
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

		return (double)tot_bytes / (INTERVAL * 0.001);
	}

	void UpSpeedEstimater::update()
	{
		if (!written_bytes.empty())
		{
			upload_rate = 0;
			upload_rate = rate(written_bytes);
		}

		
#ifdef MEASURE_PROTO_OVERHEAD
		if (!proto_bytes.empty())
		{
			proto_upload_rate = 0;
			proto_upload_rate = rate(proto_bytes);
		}
#endif
	}

}
