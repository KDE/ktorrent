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


using namespace kt;

namespace bt
{

	QueueManager::QueueManager() : QObject(),exiting(false)
	{
		downloads.setAutoDelete(true);
		max_downloads = 0;
		max_seeds = 0; //for testing. Needs to be added to Settings::

		keep_seeding = true; //test. Will be passed from Core
		paused_state = false;
	}


	QueueManager::~QueueManager()
	{}

	void QueueManager::append(kt::TorrentInterface* tc)
	{
		downloads.append(tc);
		downloads.sort();
		
		connect(tc, SIGNAL(diskSpaceLow(kt::TorrentInterface*, bool)), this, SLOT(onLowDiskSpace(kt::TorrentInterface*, bool)));
		connect(tc, SIGNAL(torrentStopped(kt::TorrentInterface*)), this, SLOT(torrentStopped(kt::TorrentInterface*)));
	}

	void QueueManager::remove(kt::TorrentInterface* tc)
	{
		paused_torrents.erase(tc);
					
		int index = downloads.findRef(tc);

		if (index != -1)
			downloads.remove(index);
		else
			Out(SYS_GEN | LOG_IMPORTANT) << "Could not delete removed torrent control." << endl;
	}

	void QueueManager::clear()
	{
		Uint32 nd = downloads.count();
		
		paused_torrents.clear();
		downloads.clear();

		// wait for a second to allow all http jobs to send the stopped event
		if (nd > 0)
			SynchronousWait(1000);
	}

	kt::TorrentStartResponse QueueManager::start(kt::TorrentInterface* tc, bool user)
	{
		const TorrentStats & s = tc->getStats();

		bool start_tc = user;

		bool check_done = false;

		if (tc->isCheckingData(check_done) && !check_done)
			return kt::BUSY_WITH_DATA_CHECK;

		if (!user)
		{
			if (s.completed)
				start_tc = (max_seeds == 0 || getNumRunning(false, true) < max_seeds);
			else
				start_tc = (max_downloads == 0 || getNumRunning(true) < max_downloads);
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
				bool shortDiskSpace = !tc->checkDiskSpace(false);

				if (shortDiskSpace)
				{
					//we're short!

					switch (Settings::startDownloadsOnLowDiskSpace())
					{

						case 0: //don't start!
							tc->setPriority(0);
							return kt::NOT_ENOUGH_DISKSPACE;

						case 1: //ask user
							if (KMessageBox::questionYesNo(0, i18n("You don't have enough disk space to download this torrent. Are you sure you want to continue?"), i18n("Insufficient disk space for %1").arg(s.torrent_name)) == KMessageBox::No)
							{
								tc->setPriority(0);
								return kt::USER_CANCELED;
							}
							else
								break;

						case 2: //force start
							break;
					}
				}
			}

			Out(SYS_GEN | LOG_NOTICE) << "Starting download" << endl;

			float ratio = kt::ShareRatio(s);

			float max_ratio = tc->getMaxShareRatio();

			if (s.completed && max_ratio > 0 && ratio >= max_ratio)
			{
				if (KMessageBox::questionYesNo(0, i18n("Torrent \"%1\" has reached its maximum share ratio. Ignore the limit and start seeding anyway?").arg(s.torrent_name), i18n("Maximum share ratio limit reached.")) == KMessageBox::Yes)
				{
					tc->setMaxShareRatio(0.00f);
					startSafely(tc);
				}
				else
					return kt::USER_CANCELED;
			}
			else
				startSafely(tc);
		}
		else
		{
			return kt::QM_LIMITS_REACHED;
		}
		
		return kt::START_OK;
	}

	void QueueManager::stop(kt::TorrentInterface* tc, bool user)
	{
		bool check_done = false;

		if (tc->isCheckingData(check_done) && !check_done)
			return;

		const TorrentStats & s = tc->getStats();

		if (s.running)
		{
			stopSafely(tc, user);
		}

		if (user) //dequeue it
			tc->setPriority(0);		
	}

	void QueueManager::startall(int type)
	{
		QPtrList<kt::TorrentInterface>::iterator i = downloads.begin();

		while (i != downloads.end())
		{
			kt::TorrentInterface* tc = *i;

			if (type >= 3)
				start(tc, true);
			else
			{
				if ((tc->getStats().completed && type == 2) || (!tc->getStats().completed && type == 1) || (type == 3))
					start(tc, true);
			}

			i++;
		}
	}

	void QueueManager::stopall(int type)
	{
		QPtrList<kt::TorrentInterface>::iterator i = downloads.begin();

		while (i != downloads.end())
		{
			kt::TorrentInterface* tc = *i;

			const TorrentStats & s = tc->getStats();

			if (tc->getStats().running)
			{
				try
				{
					if (type >= 3)
						stopSafely(tc, true);
					else if ((s.completed && type == 2) || (!s.completed && type == 1))
						stopSafely(tc, true);
				}
				catch (bt::Error & err)
				{
					QString msg =
						i18n("Error stopping torrent %1 : %2")
						.arg(s.torrent_name).arg(err.toString());
					KMessageBox::error(0, msg, i18n("Error"));
				}
			}
			else //if torrent is not running but it is queued we need to make it user controlled
				if ((s.completed && type == 2) || (!s.completed && type == 1) || (type == 3))
					tc->setPriority(0);

			i++;
		}
	}

	void QueueManager::onExit(WaitJob* wjob)
	{
		exiting = true;
		QPtrList<kt::TorrentInterface>::iterator i = downloads.begin();

		while (i != downloads.end())
		{
			kt::TorrentInterface* tc = *i;

			if (tc->getStats().running)
			{
				stopSafely(tc, false, wjob);
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
		int nr = 0;
		QPtrList<TorrentInterface>::const_iterator i = downloads.begin();

		while (i != downloads.end())
		{
			if (!(*i)->getStats().completed)
				++nr;

			++i;
		}

		return nr;
	}

	int QueueManager::countSeeds()
	{
		int nr = 0;
		QPtrList<TorrentInterface>::const_iterator i = downloads.begin();

		while (i != downloads.end())
		{
			if ((*i)->getStats().completed)
				++nr;

			++i;
		}

		return nr;
	}

	int QueueManager::getNumRunning(bool onlyDownload, bool onlySeed)
	{
		int nr = 0;
		// int test = 1;
		QPtrList<TorrentInterface>::const_iterator i = downloads.begin();

		while (i != downloads.end())
		{
			const TorrentInterface* tc = *i;

			const TorrentStats & s = tc->getStats();

			//Out() << "Torrent " << test++ << s.torrent_name << " priority: " << tc->getPriority() << endl;
			if (s.running)
			{
				if (onlyDownload)
				{
					if (!s.completed) nr++;
				}
				else
				{
					if (onlySeed)
					{
						if (s.completed) nr++;
					}
					else
						nr++;
				}
			}

			i++;
		}

		// Out() << endl;
		return nr;
	}

	int QueueManager::getNumRunning(bool userControlled, bool onlyDownloads, bool onlySeeds)
	{
		int nr = 0;
		// int test = 1;
		QPtrList<TorrentInterface>::const_iterator i = downloads.begin();

		while (i != downloads.end())
		{
			const TorrentInterface* tc = *i;

			const TorrentStats & s = tc->getStats();

			//Out() << "Torrent " << test++ << s.torrent_name << " priority: " << tc->getPriority() << endl;
			if (s.running)
			{
				if (onlyDownloads)
				{
					if (!s.completed && (userControlled && s.user_controlled)) nr++;
				}
				else
				{
					if (onlySeeds)
					{
						if (s.completed && (userControlled && s.user_controlled)) nr++;
					}
					else
						if (userControlled && s.user_controlled) nr++;
				}
			}

			i++;
		}

		// Out() << endl;
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

	void QueueManager::mergeAnnounceList(const SHA1Hash & ih, const TrackerTier* trk)

	{
		QPtrList<kt::TorrentInterface>::iterator itr = downloads.begin();

		while (itr != downloads.end())
		{
			TorrentControl* tor = (TorrentControl*)(*itr);

			if (tor->getTorrent().getInfoHash() == ih)
			{
				TrackersList* ta = tor->getTrackersList();
				ta->merge(trk);
				return;
			}

			itr++;
		}
	}

	void QueueManager::orderQueue()
	{
		if (!downloads.count())
			return;

		if (paused_state || exiting)
			return;

		downloads.sort();

		QPtrList<TorrentInterface>::const_iterator it = downloads.begin();
		QPtrList<TorrentInterface>::const_iterator its = downloads.end();


		if (max_downloads != 0 || max_seeds != 0)
		{
			bt::QueuePtrList download_queue;
			bt::QueuePtrList seed_queue;

			int user_downloading = 0;
			int user_seeding = 0;

			for (; it != downloads.end(); ++it)
			{
				TorrentInterface* tc = *it;

				const TorrentStats & s = tc->getStats();

				if (s.running && s.user_controlled)
				{
					if (!s.completed)
						++user_downloading;
					else
						++user_seeding;
				}

				if (!s.user_controlled && !tc->isMovingFiles() && !s.stopped_by_error)
				{
					if (s.completed)
						seed_queue.append(tc);
					else
						download_queue.append(tc);
				}
			}

			int max_qm_downloads = max_downloads - user_downloading;

			int max_qm_seeds = max_seeds - user_seeding;

			//stop all QM started torrents

			for (Uint32 i = max_qm_downloads; i < download_queue.count() && max_downloads; ++i)
			{
				TorrentInterface* tc = download_queue.at(i);

				const TorrentStats & s = tc->getStats();

				if (s.running && !s.user_controlled && !s.completed)
				{
					Out(SYS_GEN | LOG_DEBUG) << "QM Stopping: " << s.torrent_name << endl;
					stop(tc);
				}
			}

			//stop all QM started torrents
			for (Uint32 i = max_qm_seeds; i < seed_queue.count() && max_seeds; ++i)
			{
				TorrentInterface* tc = seed_queue.at(i);

				const TorrentStats & s = tc->getStats();

				if (s.running && !s.user_controlled && s.completed)
				{
					Out(SYS_GEN | LOG_NOTICE) << "QM Stopping: " << s.torrent_name << endl;
					stop(tc);
				}
			}

			//Now start all needed torrents
			if (max_downloads == 0)
				max_qm_downloads = download_queue.count();

			if (max_seeds == 0)
				max_qm_seeds = seed_queue.count();

			int counter = 0;

			for (Uint32 i = 0; counter < max_qm_downloads && i < download_queue.count(); ++i)
			{
				TorrentInterface* tc = download_queue.at(i);

				const TorrentStats & s = tc->getStats();

				if (!s.running && !s.completed && !s.user_controlled)
				{
					start(tc, false);

					if (tc->getStats().stopped_by_error)
					{
						tc->setPriority(0);
						continue;
					}
				}

				++counter;
			}

			counter = 0;

			for (Uint32 i = 0; counter < max_qm_seeds && i < seed_queue.count(); ++i)
			{
				TorrentInterface* tc = seed_queue.at(i);

				const TorrentStats & s = tc->getStats();

				if (!s.running && s.completed && !s.user_controlled)
				{
					start(tc, false);

					if (tc->getStats().stopped_by_error)
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

			for (it = downloads.begin(); it != downloads.end(); ++it)
			{
				TorrentInterface* tc = *it;

				const TorrentStats & s = tc->getStats();

				if (!s.running && !s.user_controlled && !s.stopped_by_error && !tc->isMovingFiles())
				{
					start(tc, false);
					if (tc->getStats().stopped_by_error)
						tc->setPriority(0);
				}
			}
		}

	}

	void QueueManager::torrentFinished(kt::TorrentInterface* tc)
	{
		//dequeue this tc
		tc->setPriority(0);
		//make sure the max_seeds is not reached
//   if(max_seeds !=0 && max_seeds < getNumRunning(false,true))
//    tc->stop(true);

		if (keep_seeding)
		    torrentAdded(tc, false, false);
		else
			stop(tc,true);

		orderQueue();
	}

	void QueueManager::torrentAdded(kt::TorrentInterface* tc, bool user, bool start_torrent)
	{
		if (!user)
		{
			QPtrList<TorrentInterface>::const_iterator it = downloads.begin();

			while (it != downloads.end())
			{
				TorrentInterface* _tc = *it;
				int p = _tc->getPriority();

				if (p == 0)
					break;
				else
					_tc->setPriority(++p);

				++it;
			}

			tc->setPriority(1);
		}
		else
		{
			tc->setPriority(0);
			if(start_torrent)
				start(tc, true);
		}

		orderQueue();
	}

	void QueueManager::torrentRemoved(kt::TorrentInterface* tc)
	{
		remove(tc);

		orderQueue();
	}

	void QueueManager::setPausedState(bool pause)
	{
		paused_state = pause;
		if (!pause)
		{
			std::set<kt::TorrentInterface*>::iterator it = paused_torrents.begin();
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
			QPtrList<TorrentInterface>::const_iterator it = downloads.begin();
			for (; it != downloads.end(); it++)
			{
				TorrentInterface* tc = *it;

				const TorrentStats & s = tc->getStats();

				if (s.running)
				{
					paused_torrents.insert(tc);
					stopSafely(tc, false);
				}
			}
		}
	}

	void QueueManager::enqueue(kt::TorrentInterface* tc)
	{
		//if a seeding torrent reached its maximum share ratio or maximum seed time don't enqueue it...
		if (tc->getStats().completed && (tc->overMaxRatio() || tc->overMaxSeedTime()))
		{
			Out(SYS_GEN | LOG_IMPORTANT) << "Torrent has reached max share ratio or max seed time and cannot be started automatically." << endl;
			emit queuingNotPossible(tc);
			return;
		}

		torrentAdded(tc, false, false);
	}

	void QueueManager::dequeue(kt::TorrentInterface* tc)
	{
		int tp = tc->getPriority();
		bool completed = tc->getStats().completed;
		QPtrList<TorrentInterface>::const_iterator it = downloads.begin();

		while (it != downloads.end())
		{
			TorrentInterface* _tc = *it;
			bool _completed = _tc->getStats().completed;

			if (tc == _tc || (_completed != completed))
			{
				++it;
				continue;
			}

			int p = _tc->getPriority();

			if (p < tp)
				break;
			else
				_tc->setPriority(--p);

			++it;
		}

		tc->setPriority(0);

		orderQueue();
	}

	void QueueManager::queue(kt::TorrentInterface* tc)
	{
		if (tc->getPriority() == 0)
			enqueue(tc);
		else
			dequeue(tc);
	}

	void QueueManager::startSafely(kt::TorrentInterface* tc)
	{
		try
		{
			tc->start();
		}
		catch (bt::Error & err)
		{
			const TorrentStats & s = tc->getStats();

			QString msg =
				i18n("Error starting torrent %1 : %2")
				.arg(s.torrent_name).arg(err.toString());

			KMessageBox::error(0, msg, i18n("Error"));
		}
	}

	void QueueManager::stopSafely(kt::TorrentInterface* tc, bool user, WaitJob* wjob)
	{
		try
		{
			tc->stop(user, wjob);
		}
		catch (bt::Error & err)
		{
			const TorrentStats & s = tc->getStats();

			QString msg =
				i18n("Error stopping torrent %1 : %2")
				.arg(s.torrent_name).arg(err.toString());

			KMessageBox::error(0, msg, i18n("Error"));
		}
	}
	
	void QueueManager::onLowDiskSpace(kt::TorrentInterface* tc, bool toStop)
	{
		if(toStop)
		{
			stop(tc, false);
		}
		
		//then emit the signal to inform trayicon to show passive popup
		emit lowDiskSpace(tc, toStop);
	}
	
	void QueueManager::torrentStopped(kt::TorrentInterface* tc )
	{
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

		if (tc1->getPriority() == tc2->getPriority())
			return 0;

		if (tc1->getPriority() == 0 && tc2->getPriority() != 0)
			return 1;
		else if (tc1->getPriority() != 0 && tc2->getPriority() == 0)
			return -1;

		return tc1->getPriority() > tc2->getPriority() ? -1 : 1;

		return 0;
	}	
}

#include "queuemanager.moc"
