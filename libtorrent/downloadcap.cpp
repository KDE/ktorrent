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
		if (rinterval == 0)
		{
			rinterval = (int)floor(((double)MAX_PIECE_LEN / cap) * 1000.0);
		}
		else
		{	
			int diff = pd->getDownloadRate() - cap;
	
			if (diff > 3*1024)
				rinterval += 200;
			else if (diff > 1024)
				rinterval += 100;
			else if (diff > 0)
				rinterval += 10;
			else if (diff < -3*1024)
				rinterval -= 200;
			else if (diff < -1024)
				rinterval -= 100;
			else if (diff < 0)
				rinterval -= 10;
			
			if (rinterval < 0)
				rinterval = 1;
		}
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

		// allow one downloader for every 5 KB/sec
		// + 2 additional to avoid slow downloaders to not bog us down
		int max_downers = max_bytes_per_sec / (5*1024) + 2;
		if (max_downers < 3)
			max_downers = 3;
		
		int num = 0;
		// the normal speed a PeerDownloader is allowed to go
		float max_speed_per_pd = 0.0f;
		if (pdowners.size() < max_downers)
		{
			num = pdowners.size();
			max_speed_per_pd = max_bytes_per_sec / (float)(num);
		}
		else
		{
			num = max_downers;
			max_speed_per_pd = max_bytes_per_sec / (float)(num - 2);
		}

		// sort by download speed
		pdowners.sort(PeerDownloadRateCmp());
		
		std::list<PeerDownloader*>::iterator i = pdowners.begin();
		int j = 0;
		while (i != pdowners.end() && j < max_downers)
		{
			// cap the max_downers first at max_speed_per_pd
			// and block the others, that way we get the fastest downloaders
			PeerDownloader* pd = *i;
			if (j < max_downers)
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

	Uint32 DownloadCap::currentSpeed()
	{
		Uint32 s = 0;
		std::list<PeerDownloader*>::iterator i = pdowners.begin();
		while (i != pdowners.end())
		{
			// cap the 4 first at max_speed_per_pd
			// and the other at 30 seconds
			PeerDownloader* pd = *i;
			s += pd->getDownloadRate();
			i++;
		}
		return s;
	}
}
