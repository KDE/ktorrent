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
			QPtrList<PeerDownloader>::iterator i = pdowners.begin();
			while (i != pdowners.end())
			{
				PeerDownloader* pd = *i;
				pd->setRequestInterval(0);
				i++;
			}
		}
	}
	
	void DownloadCap::capPD(PeerDownloader* pd,Uint32 cap)
	{
		// each piece is MAX_PIECE_LEN large
		// so the interval is (MAX_PIECE_LEN / max_speed_per_pd)
		// The * 1000 is to convert it to milliseconds
		Uint32 rti = (int)floor((MAX_PIECE_LEN / cap) * 1000.0f);
		if (pd->getDownloadRate() > cap + 3*1024)
			pd->setRequestInterval(4*rti);
		else if (pd->getDownloadRate() > cap)
			pd->setRequestInterval(2*rti);
		else
			pd->setRequestInterval(rti);
	}

	int DownloadCap::numActiveDownloaders()
	{
		int num_active=0;
		QPtrList<PeerDownloader>::iterator i = pdowners.begin();
		while (i != pdowners.end())
		{
			PeerDownloader* pd = *i;
			if (pd->getNumGrabbed() != 0)
				num_active++;
			i++;
		}
		return num_active;
	}

	void DownloadCap::capAll(float max_speed_per_pd)
	{
		// set everybody to max_speed_per_pd speed
		QPtrList<PeerDownloader>::iterator i = pdowners.begin();
		while (i != pdowners.end())
		{
			PeerDownloader* pd = *i;
			capPD(pd,(int)floor(max_speed_per_pd));
			i++;
		}
	}

	void DownloadCap::calcExcess(float max_speed_per_pd,float & exc_bw,int & num_maxed_out)
	{
		exc_bw = 0.0f;
		num_maxed_out = 0;
		// first see who is to slow
		QPtrList<PeerDownloader>::iterator i = pdowners.begin();
		while (i != pdowners.end())
		{
			PeerDownloader* pd = *i;
			if (pd->getNumGrabbed() != 0)
			{
				float dl = (float)pd->getDownloadRate();
				if (dl < max_speed_per_pd)
					exc_bw += max_speed_per_pd - dl;
				else
					num_maxed_out++;
			}
			i++;
		}
	}

	void DownloadCap::update()
	{
		if (max_bytes_per_sec == 0)
			return;
		/*	
		QPtrList<PeerDownloader>::iterator i = pdowners.begin();
		while (i != pdowners.end())
		{
			PeerDownloader* pd = *i;
			capPD(pd,max_bytes_per_sec);
			i++;
		}
		*/

		int num_active = numActiveDownloaders();
		// the normal speed a PeerDownloader is allowed to do
		float max_speed_per_pd = max_bytes_per_sec / (float)num_active;
		// cap everybody to max_speed_per_pd
		capAll(max_speed_per_pd);

		// we can give some bandwith of the slow downloaders to the fast
		// the bandwith which isn't used by slow downloaders
		float excess_bandwith = 0;
		int num_maxed_out = 0;
		// first calculate the amount of excess bandwith
		// and the number of maxed out downloaders
		calcExcess(max_speed_per_pd,excess_bandwith,num_maxed_out);
		
		if (num_maxed_out == 0)
			return;

		// increase the bandwith of fast downloaders
		QPtrList<PeerDownloader>::iterator i = pdowners.begin();
		while (i != pdowners.end())
		{
			PeerDownloader* pd = *i;
			if (pd->getNumGrabbed() != 0)
			{
				float dl = (float)pd->getDownloadRate();
				if (!(dl < max_speed_per_pd))
				{
					float s = max_speed_per_pd + excess_bandwith / num_maxed_out;
					capPD(pd,(int)floor(s));
				}
			}
			i++;
		}
		
	}
	
	void DownloadCap::addPeerDonwloader(PeerDownloader* pd)
	{
		pdowners.append(pd);
		if (max_bytes_per_sec == 0)
			pd->setRequestInterval(0);
		else
			// initially set to 1000 to not get high download peaks
			// after a tracker request was recieved
			// and we start connecting to a lot of peers
			pd->setRequestInterval(1000);
	}
	
	void DownloadCap::removePeerDownloader(PeerDownloader* pd)
	{
		pdowners.remove(pd);
	}

}
