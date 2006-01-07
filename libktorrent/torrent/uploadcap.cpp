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
#include <util/log.h>
#include <torrent/globals.h>
#include "uploadcap.h"
#include "peer.h"
#include "packetwriter.h"

namespace bt
{
	typedef QMap<PacketWriter*,QValueList<Uint32> >::iterator ByteMapItr;
	
	UploadCap UploadCap::self;

	UploadCap::UploadCap()
	{
		max_bytes_per_sec = 0;
		leftover = 0;
	}

	UploadCap::~UploadCap()
	{
	}

	void UploadCap::setMaxSpeed(Uint32 max)
	{
		max_bytes_per_sec = max;
		// tell everybody to go wild
		if (max_bytes_per_sec == 0)
		{
			QValueList<Entry>::iterator i = up_queue.begin();
			while (i != up_queue.end())
			{
				Entry & e = *i;
				e.pw->uploadUnsentPacket(true);
				i++;
			}
			up_queue.clear();
		}
	}

	bool UploadCap::allow(PacketWriter* pd,Uint32 bytes)
	{
		if (max_bytes_per_sec == 0)
		{
			timer.update();
			return true;
		}

		// append pd to queue
		Entry e;
		e.bytes = bytes;
		e.pw = pd;
		up_queue.append(e);
		return false;
	}

	void UploadCap::killed(PacketWriter* pd)
	{
		QValueList<Entry>::iterator i = up_queue.begin();
		while (i != up_queue.end())
		{
			Entry & e = *i;
			if (e.pw == pd)
				i = up_queue.erase(i);
			else
				i++;
		}
	}

	void UploadCap::update()
	{
		if (up_queue.count() == 0)
			return;
		
		// first calculate the time since the last update
		double el = timer.getElapsedSinceUpdate();
		
		// calculate the number of bytes we can send, including those leftover from the last time
		Uint32 nb = (Uint32)floor((el / 1000.0) * max_bytes_per_sec) + leftover;
		leftover = 0;
	//	Out() << "nb = " << nb << endl;
		
		bool sent_one = false;

		while (up_queue.count() > 0 && nb > 0)
		{
			// get the first
			Entry & e = up_queue.first();
			PacketWriter* pw = e.pw;
			
			if (pw->getNumPacketsToWrite() > 0 )
			{
				if (e.bytes <= nb)
				{
					// upload one
					pw->uploadUnsentPacket(false);
					nb -= e.bytes;
					sent_one = true;
					up_queue.pop_front();
				}
				else
				{
					// make sure we exit the loop
					leftover = nb;
					nb = 0;
				}
			}
			else
			{
				up_queue.pop_front();
			}
		}
		
		if (sent_one)
			timer.update();
	}
}
