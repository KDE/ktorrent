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
	}

	bool UploadCap::allow(PacketWriter* pd,Uint32 bytes)
	{
		if (max_bytes_per_sec == 0)
		{
			timer.update();
			return true;
		}

		// first update map or add new entry
		up_bytes[pd].append(bytes);

		// append pd to queue
		up_queue.append(pd);
		return false;
	}

	void UploadCap::killed(PacketWriter* pd)
	{
		up_queue.remove(pd);
		up_bytes.erase(pd);
	}

	void UploadCap::update()
	{
		// first calculate the allowwed bytes
		Uint32 el = timer.getElapsedSinceUpdate();
		float secs = el / 1000.0f;
		if (secs > 3.0f)
			secs = 3.0f;
		
		Uint32 allowed_bytes = (Uint32)floor(max_bytes_per_sec * secs);
		timer.update();

		while (allowed_bytes > 0 && up_queue.count() != 0)
		{
			// get the first
			PacketWriter* pw = up_queue.first();
			// Find out how much it wants to write
			QValueList<Uint32> & vl = up_bytes[pw];
			Uint32 num_want = vl.first();
			// only allow one chunk at a time
			if (num_want > MAX_PIECE_LEN)
				num_want = MAX_PIECE_LEN;
			// make sure we don't send more then we're allowwed
			if (num_want > allowed_bytes)
				num_want = allowed_bytes;

			num_want = pw->uploadUnsentBytes(num_want);
			vl.first() -= num_want;
			// if we sent a full packet remove the pw from the queue
			if (vl.first() == 0)
			{
				up_queue.removeFirst();
				vl.pop_front();
			}
			allowed_bytes -= num_want;
		}
	}
}
