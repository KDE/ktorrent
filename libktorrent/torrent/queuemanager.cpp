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

#include <qstring.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <util/log.h>
#include <util/error.h>
#include <util/sha1hash.h>
#include <util/waitjob.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>
#include <torrent/torrentcontrol.h>
#include <interfaces/torrentinterface.h>

using namespace kt;

namespace bt
{

	QueueManager::QueueManager() : QObject()
	{
		downloads.setAutoDelete(true);
		max_downloads = 0;
		max_seeds = 0; //for testing. Needs to be added to Settings::
		
		keep_seeding = true; //test. Will be passed from Core
	}


	QueueManager::~QueueManager()
	{}

	void QueueManager::append(kt::TorrentInterface* tc)
	{
		downloads.append(tc);
		downloads.sort();
	}

	void QueueManager::remove(kt::TorrentInterface* tc)
	{
		int index = downloads.findRef(tc);
		if(index != -1)
			downloads.remove(index);
		else
			Out() << "Could not delete removed torrent control." << endl;
	}

	void QueueManager::clear()
	{
		Uint32 nd = downloads.count();
		downloads.clear();
		
		// wait for a second to allow all http jobs to send the stopped event
		if (nd > 0)
			SynchronousWait(250);
	}

	void QueueManager::start(kt::TorrentInterface* tc)
	{
		const TorrentStats & s = tc->getStats();
		bool start_tc = false;
		if (s.completed)
			start_tc = (max_seeds == 0 || getNumRunning(false, true) < max_seeds);
		else 
	    	start_tc = (max_downloads == 0 || getNumRunning(true) < max_downloads);
		
		if (start_tc)
		{
			Out() << "Starting download" << endl;
			try
			{
				tc->start();
			}
			catch (bt::Error & err)
			{
				QString msg =
						i18n("Error starting torrent %1 : %2")
						.arg(s.torrent_name).arg(err.toString());
				KMessageBox::error(0,msg,i18n("Error"));
			}
		}
	}

	void QueueManager::stop(kt::TorrentInterface* tc, bool user)
	{
		const TorrentStats & s = tc->getStats();
		if (s.started && s.running)
		{
			try
			{
				tc->stop(user);
				if(user)
					tc->setPriority(0);
			}
			catch (bt::Error & err)
			{
				QString msg =
						i18n("Error stopping torrent %1 : %2")
						.arg(s.torrent_name).arg(err.toString());
				KMessageBox::error(0,msg,i18n("Error"));
			}
		}
		
		orderQueue();
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
			else //if torrent is not running but it is queued we need to make it user controlled
				tc->setPriority(0); 
			i++;
		}
	}
	
	void QueueManager::startNext()
	{
		orderQueue();
	}

	int QueueManager::getNumRunning(bool onlyDownload, bool onlySeed)
	{
		int nr = 0;
	//	int test = 1;
		QPtrList<TorrentInterface>::const_iterator i = downloads.begin();
		while (i != downloads.end())
		{
			const TorrentInterface* tc = *i;
			const TorrentStats & s = tc->getStats();
			//Out() << "Torrent " << test++ << s.torrent_name << " priority: " << tc->getPriority() << endl;
			if (s.running)
			{
				if(onlyDownload)
				{
					if(!s.completed) nr++;
				}
				else
				{
					if(onlySeed)
					{
						if(s.completed) nr++;
					}
					else
						nr++;
				}
			}
			i++;
		}
	//	Out() << endl;
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
	
	bool QueueManager::allreadyLoaded(const SHA1Hash & ih) const
	{
		QPtrList<kt::TorrentInterface>::const_iterator itr = downloads.begin();
		while (itr != downloads.end())
		{
			const TorrentControl* tor = (const TorrentControl*)(*itr);
			if (tor->getTorrent().getInfoHash() == ih)
				return true;
			itr++;
		}
		return false;
	}
	
	void QueueManager::orderQueue()
	{
		downloads.sort();
		
		int num_running = 0;
		
		QPtrList<TorrentInterface>::const_iterator it = downloads.begin();
		QPtrList<TorrentInterface>::const_iterator end_queue = downloads.end();
		
		if(max_downloads != 0)
		{
			int user_running = 0;
			for( ; it!=downloads.end(); ++it)
			{
				TorrentInterface* tc = *it;
				const TorrentStats & s = tc->getStats();
			
				if(s.running)
				{
					if(s.user_controlled && !s.completed)
						++user_running;
				}
			}
			
			int max_qm_downloads = max_downloads - user_running;
			//update QM boundary
			end_queue = downloads.begin();
			for(int i=0; end_queue!=downloads.end() && i<max_qm_downloads; ++i, ++end_queue);
			//stop all QM started torrents
			for(it = end_queue; it != downloads.end(); ++it)
			{
				TorrentInterface* tc = *it;
				const TorrentStats & s = tc->getStats();
				
				if(s.running && !s.user_controlled && !s.completed)
					stop(tc);
			}
		}
		
		it = downloads.begin();
		while (it != end_queue) //then check if some torrent needs to be started 
		{ 
			TorrentInterface* tc = *it; 
			const TorrentStats & s = tc->getStats(); 
			
			if(!s.running && !s.completed && !s.user_controlled)
				start(tc); 
             
			++it;
		}
		
// 		if(it == downloads.end())
// 			return;
// 		
// 		QPtrList<TorrentInterface>::const_iterator end_queue = it;
// 		
// 		while (it != downloads.end()) //first stop all torrents that aren't supposed to be running
// 		{
// 			TorrentInterface* tc = *it;
// 			const TorrentStats & s = tc->getStats();
// 			
// 			if(s.running && !s.completed && s.autostart)
// 				stop(tc);
// 			
// 			++it;
// 		}
// 		
// 		it = downloads.begin();
// 		
// 		while (it != end_queue) //then check if some torrent needs to be started
// 		{
// 			TorrentInterface* tc = *it;
// 			const TorrentStats & s = tc->getStats();
// 			
// 			if(!s.running && !s.completed && s.autostart)
// 				start(tc);
// 			
// 			++it;
// 		}
	}
	
	
	void QueueManager::torrentFinished(kt::TorrentInterface* tc)
	{
		//dequeue this tc
		tc->setPriority(0);
		//make sure the max_seeds is not reached
		Out() << "GNR Seed" << getNumRunning(false,true) << endl;
		if(max_seeds !=0 && max_seeds < getNumRunning(false,true))
			tc->stop(true);
		
		orderQueue();
	}
	
	void QueueManager::torrentAdded(kt::TorrentInterface* tc)
	{
		QPtrList<TorrentInterface>::const_iterator it = downloads.begin();
		while (it != downloads.end())
		{
			TorrentInterface* _tc = *it;
			int p = _tc->getPriority();
			if(p==0)
				break;
			else
				_tc->setPriority(++p);
			
			++it;
		}
		tc->setPriority(1);
		orderQueue();
	}
	
	void QueueManager::torrentRemoved(kt::TorrentInterface* tc)
	{
		remove(tc);
		orderQueue();
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////////

	
	QueuePtrList::QueuePtrList() : QPtrList<kt::TorrentInterface>()
	{}
	
	QueuePtrList::~QueuePtrList()
	{}
	
	int QueuePtrList::compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2)
	{
		kt::TorrentInterface* tc1 = (kt::TorrentInterface*) item1;
		kt::TorrentInterface* tc2 = (kt::TorrentInterface*) item2;
		
		if(tc1->getPriority() == tc2->getPriority())
			return 0;
		
		if(tc1->getPriority() == 0 && tc2->getPriority() != 0)
			return 1;
		else if(tc1->getPriority() != 0 && tc2->getPriority() == 0)
			return -1;
		
		return tc1->getPriority() > tc2->getPriority() ? -1 : 1;
		return 0;
	}
}

