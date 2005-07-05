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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <math.h>
#include "downloadcap.h"

namespace bt
{
	
	Uint32 DownloadCap::max_bytes_per_sec = 40*1024;
	Uint32 DownloadCap::current_speed = 0;
	Uint32 DownloadCap::outstanding_bytes = 0;
	Timer DownloadCap::timer;

	void DownloadCap::setMaxSpeed(Uint32 max)
	{
		max_bytes_per_sec = max;
	}

	void DownloadCap::setCurrentSpeed(Uint32 s)
	{
		current_speed = s;
	}

	bool DownloadCap::allow(Uint32 bytes)
	{
		return true;
		
		if (max_bytes_per_sec == 0)
		{
			timer.update();
			return true;
		}
		
		Uint32 el = timer.getElapsedSinceUpdate();
		float secs = el / 1000.0f;
		if (secs > 3.0f)
			secs = 3.0f;
		Uint32 allowed_bytes = (Uint32)floor(max_bytes_per_sec * secs);

		timer.update();
		if (bytes < allowed_bytes)
			return true;
		else
			return false;
	}

	void DownloadCap::recieved(Uint32 bytes)
	{
		if (bytes > outstanding_bytes)
			outstanding_bytes = 0;
		else
			outstanding_bytes -= bytes;
	}
}
