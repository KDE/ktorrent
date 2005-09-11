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
#include <libutil/functions.h>
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
		e.t = GetCurrentTime();
		outstanding_bytes.append(e);
	}
		
	void UpSpeedEstimater::bytesWritten(Uint32 bytes)
	{
		QValueList<Entry>::iterator i = outstanding_bytes.begin();
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
					e.t = GetCurrentTime() - e.t;
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
		
		Uint32 tot_bytes = 0;
		Uint32 tot_time = 0;
		QValueList<Entry>::iterator i = written_bytes.begin();
		while (i != written_bytes.end())
		{
			tot_bytes += (*i).bytes;
			tot_time += (*i).t;
			i++;
		}
		written_bytes.clear();
		upload_rate = (double)tot_bytes / (tot_time * 0.001);
	}

}
