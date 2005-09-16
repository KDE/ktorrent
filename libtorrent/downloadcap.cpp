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
	
	DownloadCap::DownloadCap()
	{
		setMaxSpeed(0);
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
			std::list<PeerDownloader*>::iterator i = pdowners.begin();
			while (i != pdowners.end())
			{
				PeerDownloader* pd = *i;
				pd->setBlocked(false);
				pd->setRequestInterval(0);
				i++;
			}
		}
	}
	
	void DownloadCap::capPD(PeerDownloader* pd,Uint32 cap)
	{
		int rinterval = (int)pd->getRequestInterval();
		int diff = pd->getDownloadRate() - cap;

		if (diff > 3*1024)
			rinterval += 500;
		else if (diff > 1024)
			rinterval += 100;
		else if (diff > 0)
			rinterval += 10;
		else if (diff < -3*1024)
			rinterval -= 500;
		else if (diff < -1024)
			rinterval -= 100;
		else if (diff < 0)
			rinterval -= 10;
		
		if (rinterval < 0)
			rinterval = 0;
		pd->setRequestInterval(rinterval);
		pd->setBlocked(false);
	}

	struct PeerDownloadRateCmp
	{
		bool operator () (PeerDownloader* a,PeerDownloader* b)
		{
			return a->getPeer()->getDownloadRate() > b->getPeer()->getDownloadRate();
		}
	};


	void DownloadCap::update()
	{
		if (max_bytes_per_sec == 0)
			return;

		int num = pdowners.size() < 4 ? pdowners.size() : 4;
		// the normal speed a PeerDownloader is allowed to do
		float max_speed_per_pd = max_bytes_per_sec / (float)num;

		// sort by download speed
		pdowners.sort(PeerDownloadRateCmp());
		
		std::list<PeerDownloader*>::iterator i = pdowners.begin();
		int j = 0;
		while (i != pdowners.end() && j < 4)
		{
			// cap the 4 first at max_speed_per_pd
			// and the other at 30 seconds
			PeerDownloader* pd = *i;
			if (j < 4)
				capPD(pd,(Uint32)floor(max_speed_per_pd));
			else
				pd->setBlocked(true);
			i++; j++;
		}
	}
	
	void DownloadCap::addPeerDonwloader(PeerDownloader* pd)
	{
		pdowners.push_back(pd);
		if (max_bytes_per_sec == 0)
			pd->setRequestInterval(0);
		else
			pd->setBlocked(true);
	}
	
	void DownloadCap::removePeerDownloader(PeerDownloader* pd)
	{
		pdowners.remove(pd);
	}

}
