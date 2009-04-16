/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <klocale.h>
#include "trackerinterface.h"

namespace bt
{
	TrackerInterface::TrackerInterface(const KUrl& url) : url(url)
	{
		// default 5 minute interval
		interval = 5 * 60 * 1000;
		seeders = leechers = total_downloaded = -1;
		enabled = true;
		started = false;
		status = TRACKER_IDLE;
	}
	
	TrackerInterface::~TrackerInterface() 
	{
	}
	
	void TrackerInterface::reset() 
	{
		interval = 5 * 60 * 1000;
		status = TRACKER_IDLE;
	}


	Uint32 TrackerInterface::timeToNextUpdate() const
	{
		if (!enabled || !isStarted())
			return 0;
		else
			return interval - request_time.secsTo(QDateTime::currentDateTime());
	}
	
	QString TrackerInterface::trackerStatusString() const 
	{
		switch (status)
		{
			case TRACKER_OK: return i18n("OK");
			case TRACKER_ANNOUNCING: return i18n("Announcing");
			case TRACKER_ERROR: return i18n("Error: %1",error);
			case TRACKER_IDLE:
			default: return QString();
		}
	}

}

