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
#include <kurl.h>
#include <util/functions.h>
#include <interfaces/torrentinterface.h>
#include "tracker.h"

namespace bt
{

	Tracker::Tracker(kt::TorrentInterface* tor,
					 const SHA1Hash & ih,const PeerID & id) : tor(tor)
	{
		info_hash = ih;
		peer_id = id;
		interval = 120;
		seeders = leechers = 0;
		num_failed_attempts = 0;
		connect(&update_timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
		connect(&error_update_timer,SIGNAL(timeout()),this,SLOT(onErrorTimeout()));
		error_mode = false;
	}


	Tracker::~Tracker()
	{}

	void Tracker::start()
	{
		event = "started";
		doRequest(tor->getTrackerURL(true));
		update_timer.start(interval*1000);
		time_of_last_update = GetCurrentTime();
	}
		
	void Tracker::setInterval(Uint32 secs)
	{
		if (interval != secs)
		{
			update_timer.changeInterval(1000*secs);
			time_of_last_update = GetCurrentTime();
		}
		interval = secs;
	}
		
	void Tracker::stop()
	{
		event = "stopped";
		doRequest(tor->getTrackerURL(true));
		update_timer.stop();
	}

	void Tracker::onTimeout()
	{
		if (!error_mode)
		{
			event = QString::null;
			doRequest(tor->getTrackerURL(true));
			time_of_last_update = GetCurrentTime();
		}
	}

	void Tracker::onErrorTimeout()
	{
		doRequest(tor->getTrackerURL(false));
		time_of_last_update = GetCurrentTime();
	}

	void Tracker::updateOK()
	{
		error_mode = false;
		num_failed_attempts = 0;
		error_update_timer.stop();
	}

	void Tracker::handleError()
	{
		if (event != "stopped")
		{
			error_mode = true;
			num_failed_attempts++;
			// first try 5 times in a row
			// after 5 attempts switch to a 30 second delay
			if (num_failed_attempts < 5)
			{
				doRequest(tor->getTrackerURL(false));
				time_of_last_update = GetCurrentTime();
			}
			else
				error_update_timer.start(30*1000,true);
		}
	}

	void Tracker::manualUpdate()
	{
		event = QString::null;
		doRequest(tor->getTrackerURL(true));
		time_of_last_update = GetCurrentTime();
	}

	void Tracker::completed()
	{
		event = "completed";
		doRequest(tor->getTrackerURL(true));
	}

	Uint32 Tracker::getTimeToNextUpdate() const
	{
		Uint32 s = (GetCurrentTime() - time_of_last_update) / 1000;
		if (error_mode)
		{
			if (s > 30)
				return 0;
			else
				return 30 - s;
		}
		else
		{
			if (s > interval)
				return 0;
			else
				return interval - s;
		}
	}
}

#include "tracker.moc"
