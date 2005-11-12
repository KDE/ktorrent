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
			QPtrList<PacketWriter>::iterator i = up_queue.begin();
			while (i != up_queue.end())
			{
				PacketWriter* pw = *i;
				pw->uploadUnsentPacket(true);
				i++;
			}
			up_queue.clear();
		}
	}

	bool UploadCap::allow(PacketWriter* pd)
	{
		if (max_bytes_per_sec == 0)
		{
			timer.update();
			return true;
		}

		// append pd to queue
		up_queue.append(pd);
		return false;
	}

	void UploadCap::killed(PacketWriter* pd)
	{
		up_queue.remove(pd);
	}

	void UploadCap::update()
	{
		if (up_queue.count() == 0)
			return;
		
		// first calculate the time since the last update
		double el = timer.getElapsedSinceUpdate();
		// the interval between two uploads
		double up_interval = 1000.0 / ((double)max_bytes_per_sec / MAX_PIECE_LEN);
		// the number of packets we can send is the elapsed time divided by the interval
		Uint32 npackets = (Uint32)floor(el / up_interval);
		//if (npackets > 1)
		//	npackets == 0;
		
		bool sent_one = false;

		while (up_queue.count() > 0 && npackets > 0)
		{
			// get the first
			PacketWriter* pw = up_queue.first();
			
			if (pw->getNumPacketsToWrite() > 0)
			{
				// upload one
				pw->uploadUnsentPacket(false);
				npackets--;
				sent_one = true;
			}
			// remove the first entry
			up_queue.removeFirst();
		}
		
		if (sent_one)
			timer.update();
	}
}
