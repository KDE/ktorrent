/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include "queuemanager.h"
#include <interfaces/torrentinterface.h>

#include <util/log.h>
#include <torrent/globals.h>

using namespace kt;

namespace bt
{

	QueueManager::QueueManager()
			: QObject()
	{
		downloads.setAutoDelete(true);
		max_downloads = 0;
		max_seeds = 2; //for testing. Needs to be added to Settings::
		
		keep_seeding = true; //test. Will be passed from Core
	}


	QueueManager::~QueueManager()
	{}

	void QueueManager::append(kt::TorrentInterface* tc)
	{
		downloads.append(tc);
	}

	void QueueManager::remove(kt::TorrentInterface* tc)
	{
		downloads.remove(tc);
	}

	void QueueManager::clear()
	{
		downloads.clear();
	}

	void QueueManager::start(kt::TorrentInterface* tc)
	{
		const TorrentStats & s = tc->getStats();
		bool start_tc = (s.bytes_left == 0 && (keep_seeding && getNumRunning(false, true) < max_seeds) ||
		                (s.bytes_left != 0 &&
				(max_downloads == 0 || getNumRunning(true) < max_downloads)));
		if (start_tc)
		{
			Out() << "Starting download" << endl;
			tc->start();
		}
	}

	void QueueManager::startall()
	{
		QPtrList<kt::TorrentInterface>::iterator i = downloads.begin();
		while (i != downloads.end())
		{
			kt::TorrentInterface* tc = *i;
			start(tc);
			i++;
		}
	}

	void QueueManager::stopall()
	{
		QPtrList<kt::TorrentInterface>::iterator i = downloads.begin();
		while (i != downloads.end())
		{
			kt::TorrentInterface* tc = *i;
			if (tc->getStats().running)
				tc->stop(true);
			i++;
		}
	}

	int QueueManager::getNumRunning(bool onlyDownload, bool onlySeed)
	{
		int nr = 0;
		QPtrList<TorrentInterface>::const_iterator i = downloads.begin();
		while (i != downloads.end())
		{
			const TorrentInterface* tc = *i;
			const TorrentStats & s = tc->getStats();
			if (s.running)
			{
				if(onlyDownload)
				{
					if(s.bytes_left != 0) nr++;
				}
				else
				{
					if(onlySeed)
					{
						if(s.bytes_left == 0) nr++;
					}
					else
						nr++;
				}
			}
			i++;
		}
		return nr;
	}

	QPtrList<kt::TorrentInterface>::iterator QueueManager::begin()
	{
		return downloads.begin();
	}

	QPtrList<kt::TorrentInterface>::iterator QueueManager::end()
	{
		return downloads.end();
	}

	void QueueManager::setMaxDownloads(int m)
	{
		max_downloads = m;
	}

	void QueueManager::setMaxSeeds(int m)
	{
		max_seeds = m;
	}
	
	void QueueManager::setKeepSeeding(bool ks)
	{
		keep_seeding = ks;
	}

}

