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
#include "downloadcap.h"
#include "peer.h"
#include "peerdownloader.h"

namespace bt
{
	DownloadCap DownloadCap::self;

	const Uint32 SLOT_SIZE = 5*1024;
	
	DownloadCap::DownloadCap()
	{
		setMaxSpeed(0);
		timer.update();
	}

	DownloadCap::~ DownloadCap()
	{
	}

	//MAX_PIECE_LEN

	void DownloadCap::setMaxSpeed(Uint32 max)
	{
		max_bytes_per_sec = max;
		if (max_bytes_per_sec == 0)
		{
			req_interval = 0;
			// tell everybody to go wild
			while (dl_queue.size() > 0)
			{
				PeerDownloader* pd = dl_queue.first();
				pd->downloadUnsent();
				dl_queue.remove(pd);
			}
		}
		else
		{
			req_interval = 1000.0 / ((double)max / MAX_PIECE_LEN);
		}
	}

	bool DownloadCap::allow(PeerDownloader* pd)
	{
		if (max_bytes_per_sec == 0)
			return true;

		// add pd to the queue
		dl_queue.append(pd);
		// do an update
		update();
		return false;
	}


	void DownloadCap::killed(PeerDownloader* pd)
	{
		dl_queue.remove(pd);
	}

	void DownloadCap::update()
	{
		if (timer.getElapsedSinceUpdate() >= req_interval && dl_queue.size() > 0)
		{
			// get pd from the queue
			PeerDownloader* pd = dl_queue.first();
			dl_queue.pop_front();
			// tell it to download one
			pd->downloadOneUnsent();
			timer.update();
		}
	}
	

}
