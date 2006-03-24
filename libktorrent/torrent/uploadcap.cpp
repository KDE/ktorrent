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
	
	UploadSlot::UploadSlot() : pw(0),bytes(0) {}
	
	UploadSlot::UploadSlot(PacketWriter* pw,Uint32 bytes) : pw(pw),bytes(bytes)
	{}
	
	UploadSlot::UploadSlot(const UploadSlot & us) : pw(us.pw),bytes(us.bytes)
	{}
	
	UploadSlot::~UploadSlot()
	{}
		
	bool UploadSlot::doUpload(Uint32 nb)
	{
		pw->uploadUnsentBytes(nb);
		bytes -= nb;
		return bytes == 0;
	}
	
	//////////////////////////
	
	typedef QValueList<UploadSlot>::iterator UpQueueItr;
	
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
			UpQueueItr i = up_queue.begin();
			while (i != up_queue.end())
			{
				UploadSlot & us = *i;
				us.doUpload(0);
				i++;
			}
			up_queue.clear();
			leftover = 0;
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
		up_queue.append(UploadSlot(pd,bytes));
		return false;
	}

	void UploadCap::killed(PacketWriter* pd)
	{
		UpQueueItr i = up_queue.begin();
		while (i != up_queue.end())
		{
			UploadSlot & e = *i;
			if (e.fromPW(pd))
				i = up_queue.erase(i);
			else
				i++;
		}
	}

	void UploadCap::update()
	{
		if (up_queue.count() == 0)
		{
			timer.update();
			return;
		}
		
		// first calculate the time since the last update
		double el = timer.getElapsedSinceUpdate();
		
		// calculate the number of bytes we can send, including those leftover from the last time
		Uint32 nb = (Uint32)floor((el / 1000.0) * max_bytes_per_sec) + leftover;
		leftover = 0;
	//	Out() << "nb = " << nb << endl;
		
		while (up_queue.count() > 0 && nb > 0)
		{
			// get the first
			UploadSlot & e = up_queue.first();
			
			if (e.bytesLeft() <= nb)
			{
				nb -= e.bytesLeft();
				// we can send all remaining bytes of the packet
				e.doUpload(e.bytesLeft());
				up_queue.pop_front();
			}
			else
			{
				// sent nb bytes of the packets
				e.doUpload(nb);
				nb = 0;
			}
		}
		
		leftover = nb;
		timer.update();
	}
}
