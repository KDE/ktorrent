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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include "queuemanager.h"

#include <qstring.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <util/log.h>
#include <util/error.h>
#include <util/sha1hash.h>
#include <util/waitjob.h>
#include <util/fileops.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>
#include <torrent/torrentcontrol.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/trackerslist.h>
#include <settings.h>

using namespace bt;

namespace kt
{

	QueueManager::QueueManager() : QObject()
	{
		max_downloads = 0;
		max_seeds = 0; //for testing. Needs to be added to Settings::
		
		keep_seeding = true; //test. Will be passed from Core
		paused_state = false;
		exiting = false;
	}


	QueueManager::~QueueManager()
	{}

	void QueueManager::append(bt::TorrentInterface* tc)
	{
		downloads.append(tc);
		downloads.sort();
		connect(tc, SIGNAL(diskSpaceLow(bt::TorrentInterface*, bool)), this, SLOT(onLowDiskSpace(bt::TorrentInterface*, bool)));	
		connect(tc, SIGNAL(torrentStopped(bt::TorrentInterface*)), this, SLOT(torrentStopped(bt::TorrentInterface*)));
	}

	void QueueManager::remove(bt::TorrentInterface* tc)
	{
		paused_torrents.erase(tc);
		int index = downloads.indexOf(tc);
		if(index != -1)
			delete downloads.takeAt(index);
	}

	void QueueManager::clear()
	{
		Uint32 nd = downloads.count();
		downloads.clear();
		paused_torrents.clear();
		
		// wait for a second to allow all http jobs to send the stopped event
		if (nd > 0)
			SynchronousWait(1000);
	}

	TorrentStartResponse QueueManager::start(bt::TorrentInterface* tc, bool user)
	{
		const TorrentStats & s = tc->getStats();
		bool start_tc = user;
		bool check_done = false;

		if (tc->isCheckingData(check_done) && !check_done)
			return BUSY_WITH_DATA_CHECK;

		if (!user)
		{
			if (s.completed)
				start_tc = (max_seeds == 0 || getNumRunning(SEEDS) < max_seeds);
			else
				start_tc = (max_downloads == 0 || getNumRunning(DOWNLOADS) < max_downloads);
		}
		else
		{
			//User started this torrent so make it user controlled
			tc->setPriority(0);
		}

		if (start_tc)
		{
			if (!s.completed) //no need to check diskspace for seeding torrents
			{
				//check diskspace
				if (!tc->checkDiskSpace(false))
				{
					//we're short!
					switch (Settings::startDownloadsOnLowDiskSpace())
					{
						case 0: //don't start!
							tc->setPriority(0);
							return bt::NOT_ENOUGH_DISKSPACE;
						case 1: //ask user
							if (KMessageBox::questionYesNo(0, i18n("You don't have enough disk space to download this torrent. Are you sure you want to continue?"), i18n("Insufficient disk space for %1",s.torrent_name)) == KMessageBox::No)
							{
								tc->setPriority(0);
								return bt::USER_CANCELED;
							}
							else
								break;
						case 2: //force start
							break;
					}
				}
			}

			Out(SYS_GEN | LOG_NOTICE) << "Starting download" << endl;
			bool max_ratio_reached = tc->overMaxRatio();
			bool max_seed_time_reached = tc->overMaxSeedTime();

			if (s.completed && (max_ratio_reached || max_seed_time_reached))
			{
				QString msg; 
				if (max_ratio_reached && max_seed_time_reached)
					msg = msg = i18n("The torrent \"%1\" has reached it's maximum share ratio and it's maximum seed time. Ignore the limit and start seeding anyway?",s.torrent_name);
				else if (max_ratio_reached && !max_seed_time_reached)
					msg = i18n("The torrent \"%1\" has reached it's maximum share ratio. Ignore the limit and start seeding anyway?",s.torrent_name);
				else if (max_seed_time_reached && !max_ratio_reached)
					msg = msg = i18n("The torrent \"%1\" has reached it's maximum seed time. Ignore the limit and start seeding anyway?",s.torrent_name);
				
				if (KMessageBox::questionYesNo(0, msg, i18n("Maximum share ratio limit reached.")) == KMessageBox::Yes)
				{
					if (max_ratio_reached)
						tc->setMaxShareRatio(0.00f);
					if (max_seed_time_reached)
						tc->setMaxSeedTime(0.0f);
					startSafely(tc);
				}
				else
					return USER_CANCELED;
			}
			else
				startSafely(tc);
		}
		else
		{
			return QM_LIMITS_REACHED;
		}
		
		return START_OK;
	}

	void QueueManager::stop(bt::TorrentInterface* tc, bool user)
	{
		bool check_done = false;
		if (tc->isCheckingData(check_done) && !check_done)
			return;
		
		const TorrentStats & s = tc->getStats();
		if (s.running)
		{
			stopSafely(tc,user);
		}
		
		if(user) //dequeue it
			tc->setPriority(0);
	}
	
	void QueueManager::checkDiskSpace(QList<bt::TorrentInterface*> & todo)
	{
		// first see if we need to ask the user to start torrents when diskspace is low
		if (Settings::startDownloadsOnLowDiskSpace() == 2)
		{
			QStringList names;
			QList<bt::TorrentInterface*> tmp;
			foreach (bt::TorrentInterface* tc,todo)
			{
				const TorrentStats & s = tc->getStats();
				if (!s.completed && !tc->checkDiskSpace(false))
				{
					names.append(s.torrent_name);
					tmp.append(tc);
				}
			}
			
			if (tmp.count() > 0)
			{
				if (KMessageBox::questionYesNoList(0,i18n("Not enough disk space for the following torrents. Do you want to start them anyway ?"),names) == KMessageBox::No)
				{
					foreach (bt::TorrentInterface* tc,tmp)
						todo.removeAll(tc);
				}
			}
		}
		// if the policy is to not start, remove torrents from todo list if diskspace is low
		else if (Settings::startDownloadsOnLowDiskSpace() == 0) 
		{
			QList<bt::TorrentInterface *>::iterator i = todo.begin();
			while (i != todo.end())		
			{
				bt::TorrentInterface* tc = *i;
				const TorrentStats & s = tc->getStats();
				if (!s.completed && !tc->checkDiskSpace(false))
					i = todo.erase(i);
				else
					i++;
			}
		}
	}
	
	void QueueManager::checkMaxSeedTime(QList<bt::TorrentInterface*> & todo)
	{
		QStringList names;
		QList<bt::TorrentInterface*> tmp;
		foreach (bt::TorrentInterface* tc,todo)
		{
			const TorrentStats & s = tc->getStats();
			if (tc->overMaxSeedTime())
			{
				names.append(s.torrent_name);
				tmp.append(tc);
			}
		}
			
		if (tmp.count() > 0)
		{
			if (KMessageBox::questionYesNoList(0,i18n("The following torrents have reached their maximum seed time. Do you want to start them anyway ?"),names) == KMessageBox::No)
			{
				foreach (bt::TorrentInterface* tc,tmp)
					todo.removeAll(tc);
			}
			else
			{
				foreach (bt::TorrentInterface* tc,tmp)
					tc->setMaxSeedTime(0.0f);
			}
		}
	}
	
	void QueueManager::checkMaxRatio(QList<bt::TorrentInterface*> & todo)
	{
		QStringList names;
		QList<bt::TorrentInterface*> tmp;
		foreach (bt::TorrentInterface* tc,todo)
		{
			const TorrentStats & s = tc->getStats();
			if (tc->overMaxRatio())
			{
				names.append(s.torrent_name);
				tmp.append(tc);
			}
		}
			
		if (tmp.count() > 0)
		{
			if (KMessageBox::questionYesNoList(0,i18n("The following torrents have reached their maximum share ratio. Do you want to start them anyway ?"),names) == KMessageBox::No)
			{
				foreach (bt::TorrentInterface* tc,tmp)
					todo.removeAll(tc);
			}
			else
			{
				foreach (bt::TorrentInterface* tc,tmp)
					tc->setMaxShareRatio(0.0f);
			}
		}
	}
	
	void QueueManager::start(QList<bt::TorrentInterface*> & todo)
	{
		if (todo.count() == 0)
			return;
		
		// check dispace stuff
		checkDiskSpace(todo);
		if (todo.count() == 0)
			return;
		
		checkMaxSeedTime(todo);
		if (todo.count() == 0)
			return;
		
		checkMaxRatio(todo);
		if (todo.count() == 0)
			return;
		
		// start what is left
		foreach (bt::TorrentInterface* tc,todo)
		{
			const TorrentStats & s = tc->getStats();
			if (s.running)
				continue;
			
			bool check_done = false;
			if (tc->isCheckingData(check_done) && !check_done)
				continue;
			
			tc->setPriority(0);
			startSafely(tc);
		}
	}
	
	void QueueManager::startall(int type)
	{
		// first get the list of torrents which need to be started
		QList<bt::TorrentInterface*> todo;
		foreach (bt::TorrentInterface* tc,downloads)
		{
			const TorrentStats & s = tc->getStats();
			if (s.running)
				continue;
			
			bool check_done = false;
			if (tc->isCheckingData(check_done) && !check_done)
				continue;
			
			if ((s.completed && type == 2) || (!s.completed && type == 1) || (type == 3))
			{
				todo.append(tc);
			}
		}
		
		start(todo);
	}

	void QueueManager::stopall(int type)
	{
		QList<bt::TorrentInterface *>::iterator i = downloads.begin();
		while (i != downloads.end())
		{
			bt::TorrentInterface* tc = *i;
			const TorrentStats & s = tc->getStats();
			if (tc->getStats().running)
			{
				try
				{
					if(type >= 3)
						stopSafely(tc,true);
					else if( (s.completed && type == 2) || (!s.completed && type == 1) )
						stopSafely(tc,true);
				}
				catch (bt::Error & err)
				{
					QString msg =
							i18n("Error stopping torrent %1 : %2",
							s.torrent_name,err.toString());
					KMessageBox::error(0,msg,i18n("Error"));
				}
			}
			else //if torrent is not running but it is queued we need to make it user controlled
				if( (s.completed && type == 2) || (!s.completed && type == 1) || (type == 3) )
					tc->setPriority(0); 
			i++;
		}
	}
	
	void QueueManager::onExit(WaitJob* wjob)
	{
		exiting = true;
		QList<bt::TorrentInterface *>::iterator i = downloads.begin();
		while (i != downloads.end())
		{
			bt::TorrentInterface* tc = *i;
			if (tc->getStats().running)
			{
				stopSafely(tc,false,wjob);
			}
			i++;
		}
	}
	
	void QueueManager::startNext()
	{
		orderQueue();
	}

	int QueueManager::countDownloads()
	{
		return getNumRunning(DOWNLOADS);
	}

	int QueueManager::countSeeds()
	{
		return getNumRunning(SEEDS);
	}
	
	int QueueManager::getNumRunning(Flags flags)
	{
		int nr = 0;
		QList<TorrentInterface*>::const_iterator i = downloads.begin();
		while (i != downloads.end())
		{
			const TorrentInterface* tc = *i;
			const TorrentStats & s = tc->getStats();
			
			if (s.running)
			{
				if (flags == ALL || (flags == DOWNLOADS && !s.completed) || (flags == SEEDS && s.completed))
					nr++;
			}
			i++;
		}
		return nr;
	}
	
	const bt::TorrentInterface* QueueManager::getTorrent(Uint32 idx) const
	{
		if (idx >= (Uint32)downloads.count())
			return 0;
		else
			return downloads[idx];
	}

	QList<bt::TorrentInterface *>::iterator QueueManager::begin()
	{
		return downloads.begin();
	}

	QList<bt::TorrentInterface *>::iterator QueueManager::end()
	{
		return downloads.end();
	}

	void QueueManager::setMaxDownloads(int m)
	{
		max_downloads = m;
	}
	
	void QueueManager::onLowDiskSpace(bt::TorrentInterface* tc, bool toStop)
	{
		if(toStop)
		{
			stop(tc, false);
		}
		
		//then emit the signal to inform trayicon to show passive popup
		emit lowDiskSpace(tc, toStop);
	}

	void QueueManager::setMaxSeeds(int m)
	{
		max_seeds = m;
	}
	
	void QueueManager::setKeepSeeding(bool ks)
	{
		keep_seeding = ks;
	}
	
	bool QueueManager::allreadyLoaded(const bt::SHA1Hash & ih) const
	{
		foreach (const bt::TorrentInterface* tor,downloads)
		{
			if (tor->getInfoHash() == ih)
				return true;
		}
		return false;
	}
	
	void QueueManager::mergeAnnounceList(const bt::SHA1Hash & ih,const TrackerTier* trk)
	{
		foreach (bt::TorrentInterface* tor,downloads)
		{
			if (tor->getInfoHash() == ih)
			{
				TrackersList* ta = tor->getTrackersList(); 
				ta->merge(trk);
				return;
			}
		}
	}
	
	void QueueManager::orderQueue()
	{
		if(!downloads.count())
			return;
		
		if(paused_state || exiting)
			return;
		
		downloads.sort();
                
		QList<TorrentInterface *>::const_iterator it = downloads.begin();
		QList<TorrentInterface *>::const_iterator its = downloads.end();
		
                
		if(max_downloads != 0 || max_seeds != 0)
		{	
			QueuePtrList download_queue;
			QueuePtrList seed_queue;
			
			int user_downloading = 0;
			int user_seeding = 0;
			
			for( ; it!=downloads.end(); ++it)
			{
				TorrentInterface* tc = *it;
				const TorrentStats & s = tc->getStats();

				if(s.running && s.user_controlled)
				{
					if(!s.completed)
						++user_downloading;
					else
						++user_seeding;
				}
				
				if(!s.user_controlled && !tc->isMovingFiles() && !s.stopped_by_error)
				{
					if(s.completed)
						seed_queue.append(tc);
					else		
						download_queue.append(tc);
				}
			}

			Uint32 max_qm_downloads = max_downloads > user_downloading ? max_downloads - user_downloading : 0;
			Uint32 max_qm_seeds = max_seeds > user_seeding ? max_seeds - user_seeding : 0;
			
			//stop all QM started torrents
			for(Uint32 i=max_qm_downloads; i < (Uint32)download_queue.count() && max_downloads; ++i)
			{
				TorrentInterface* tc = download_queue.at(i);
				const TorrentStats & s = tc->getStats();
                                
				if(s.running && !s.user_controlled && !s.completed)
				{
					Out(SYS_GEN|LOG_DEBUG) << "QM Stopping: " << s.torrent_name << endl;
					stop(tc);
				}
			}
			
			//stop all QM started torrents
			for(Uint32 i=max_qm_seeds; i < (Uint32)seed_queue.count() && max_seeds; ++i)
			{
				TorrentInterface* tc = seed_queue.at(i);
				const TorrentStats & s = tc->getStats();
                                
				if(s.running && !s.user_controlled && s.completed)
				{
					Out(SYS_GEN|LOG_NOTICE) << "QM Stopping: " << s.torrent_name << endl;
					stop(tc);
				}
			}
			
			//Now start all needed torrents
			if(max_downloads == 0)
				max_qm_downloads = download_queue.count();
			
			if(max_seeds == 0) 
				max_qm_seeds = seed_queue.count();
			
			Uint32 counter = 0;
			for (Uint32 i=0; counter < max_qm_downloads && i < (Uint32)download_queue.count(); ++i)
			{
				TorrentInterface* tc = download_queue.at(i);
				const TorrentStats & s = tc->getStats();
				
				if(!s.running && !s.completed && !s.user_controlled && !s.stopped_by_error)
				{
					start(tc, false);
					if(tc->getStats().stopped_by_error)
					{
						tc->setPriority(0);
						continue;
					}
				}
				
				++counter;
			}
			
			counter = 0;
			for(Uint32 i=0; counter < max_qm_seeds && i < (Uint32)seed_queue.count(); ++i)
			{
				TorrentInterface* tc = seed_queue.at(i);
				const TorrentStats & s = tc->getStats();
				
				if(!s.running && s.completed && !s.user_controlled && !s.stopped_by_error)
				{
					start(tc, false);
					if(tc->getStats().stopped_by_error)
					{
						tc->setPriority(0);
						continue;
					}
				}
				
				++counter;
			}
		}
		else
		{
			//no limits at all
			for(it=downloads.begin(); it!=downloads.end(); ++it)
			{
				TorrentInterface* tc = *it;
				const TorrentStats & s = tc->getStats();
                        
				if(!s.running && !s.user_controlled && !tc->isMovingFiles() && !s.stopped_by_error)
				{
					start(tc, false);
					if(tc->getStats().stopped_by_error)
						tc->setPriority(0);
				}
			}
		}
		emit queueOrdered();
	}
	
	void QueueManager::torrentFinished(bt::TorrentInterface* tc)
	{
		//dequeue this tc
		tc->setPriority(0);
		//make sure the max_seeds is not reached
// 		if(max_seeds !=0 && max_seeds < getNumRunning(false,true))
// 			tc->stop(true);
		
		if (keep_seeding)
			torrentAdded(tc,false,false);
		else
			stopSafely(tc,true);
		
		orderQueue();
	}
	
	void QueueManager::torrentAdded(bt::TorrentInterface* tc,bool user, bool start_torrent)
	{
		if (!user)
		{
			QList<TorrentInterface *>::const_iterator it = downloads.begin();
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
		else
		{
			 tc->setPriority(0);
			 if(start_torrent)
				 start(tc, true);
		}	
	}
	
	void QueueManager::torrentRemoved(bt::TorrentInterface* tc)
	{
		remove(tc);
		orderQueue();
	}
	
	void QueueManager::setPausedState(bool pause)
	{
		paused_state = pause;	
		if(!pause)
		{
			std::set<bt::TorrentInterface*>::iterator it = paused_torrents.begin();
			while (it != paused_torrents.end())
			{
				TorrentInterface* tc = *it;
				startSafely(tc);
				it++;
			}
			
			paused_torrents.clear();
			orderQueue();
		}
		else
		{
			foreach (TorrentInterface* tc,downloads)
			{
				const TorrentStats & s = tc->getStats();         
				if(s.running)
				{
					paused_torrents.insert(tc);
					stopSafely(tc,false);
				}
			}
		}
		emit pauseStateChanged(paused_state);
	}
	
	void QueueManager::enqueue(bt::TorrentInterface* tc)
	{
		//if a seeding torrent reached its maximum share ratio or maximum seed time don't enqueue it...
		if (tc->getStats().completed && (tc->overMaxRatio() || tc->overMaxSeedTime()))
		{
			Out(SYS_GEN | LOG_IMPORTANT) << "Torrent has reached max share ratio or max seed time and cannot be started automatically." << endl;
			emit queuingNotPossible(tc);
			return;
		}
		
		torrentAdded(tc,false,false);
	}
	
	void QueueManager::dequeue(bt::TorrentInterface* tc)
	{
		int tp = tc->getPriority();
		bool completed = tc->getStats().completed;
		QList<TorrentInterface *>::const_iterator it = downloads.begin();
		while (it != downloads.end())
		{
			TorrentInterface* _tc = *it;
			bool _completed = _tc->getStats().completed;
			
			if(tc == _tc || (_completed != completed))
			{
				++it;
				continue;
			}
			
			int p = _tc->getPriority();
			if(p<tp)
				break;
			else
				_tc->setPriority(--p);
			
			++it;
		}
		tc->setPriority(0);
		orderQueue();
	}
	
	void QueueManager::queue(bt::TorrentInterface* tc)
	{
		if(tc->getPriority() == 0)
			enqueue(tc);
		else
			dequeue(tc);
	}
	
	void QueueManager::startSafely(bt::TorrentInterface* tc)
	{
		try
		{
			tc->start();
		}
		catch (bt::Error & err)
		{
			const TorrentStats & s = tc->getStats();
			QString msg =
					i18n("Error starting torrent %1 : %2",
					s.torrent_name,err.toString());
			KMessageBox::error(0,msg,i18n("Error"));
		}
	}
	
	void QueueManager::stopSafely(bt::TorrentInterface* tc,bool user,WaitJob* wjob)
	{
		try
		{
			tc->stop(user,wjob);
		}
		catch (bt::Error & err)
		{
			const TorrentStats & s = tc->getStats();
			QString msg =
					i18n("Error stopping torrent %1 : %2",
					s.torrent_name,err.toString());
			KMessageBox::error(0,msg,i18n("Error"));
		}
	}
	
	void QueueManager::torrentStopped(bt::TorrentInterface* )
	{
		orderQueue();
	}
	/////////////////////////////////////////////////////////////////////////////////////////////

	
	QueuePtrList::QueuePtrList() : QList<bt::TorrentInterface *>()
	{}
	
	QueuePtrList::~QueuePtrList()
	{}
	
	void QueuePtrList::sort()
	{
		qSort(begin(),end(),QueuePtrList::biggerThan);
	}
	
	bool QueuePtrList::biggerThan(bt::TorrentInterface* tc1, bt::TorrentInterface* tc2)
	{
		return tc1->getPriority() > tc2->getPriority();
	}
	
}

#include "queuemanager.moc"
