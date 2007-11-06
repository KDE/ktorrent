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
#ifndef QUEUEMANAGER_H
#define QUEUEMANAGER_H

#include <set>
#include <qobject.h>
#include <qptrlist.h>

#include <interfaces/torrentinterface.h>

namespace kt
{
	class TrackersList;
}

namespace bt
{
	class SHA1Hash;
	class AnnounceList;
	struct TrackerTier;
	class WaitJob;

	class QueuePtrList : public QPtrList<kt::TorrentInterface>
	{
		public:
			QueuePtrList();
			virtual ~QueuePtrList();

		protected:
			int compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2);
	};

	/**
	 * @author Ivan Vasic
	 * @brief This class contains list of all TorrentControls and is responsible for starting/stopping them
	 */
	class QueueManager : public QObject
	{
		Q_OBJECT
				
		public:
			QueueManager();
			virtual ~QueueManager();

			void append(kt::TorrentInterface* tc);
			void remove(kt::TorrentInterface* tc);
			void clear();
			
			kt::TorrentStartResponse start(kt::TorrentInterface* tc, bool user = true);
			void stop(kt::TorrentInterface* tc, bool user = false);

			void stopall(int type);
			void startall(int type);
			
			/**
			 * Stop all running torrents
			 * @param wjob WaitJob which waits for stopped events to reach the tracker
			 */
			void onExit(WaitJob* wjob);

			/**
			 * Enqueue/Dequeue function. Places a torrent in queue. 
			 * If the torrent is already in queue this will remove it from queue.
			 * @param tc TorrentControl pointer.
			 */
			void queue(kt::TorrentInterface* tc);

			int count() { return downloads.count(); }
			int countDownloads();
			int countSeeds();

			int getNumRunning(bool onlyDownload = false, bool onlySeed = false);
			int getNumRunning(bool userControlled, bool onlyDownloads, bool onlySeeds);

			void startNext();
			
			typedef QPtrList<kt::TorrentInterface>::iterator iterator;

			iterator begin();
			iterator end();

			/**
			 * See if we already loaded a torrent.
			 * @param ih The info hash of a torrent
			 * @return true if we do, false if we don't
			 */
			bool allreadyLoaded(const SHA1Hash & ih) const;


			/**
			 * Merge announce lists to a torrent
			 * @param ih The info_hash of the torrent to merge to
			 * @param trk First tier of trackers
			 */
			void mergeAnnounceList(const SHA1Hash & ih,const TrackerTier* trk);

			void setMaxDownloads(int m);
			void setMaxSeeds(int m);

			void setKeepSeeding(bool ks);

			/**
			 * Sets global paused state for QueueManager and stopps all running torrents.
			 * No torrents will be automatically started/stopped with QM.
			 */
			void setPausedState(bool pause);

			/**
			 * Places all torrents from downloads in the right order in queue.
			 * Use this when torrent priorities get changed
			 */
			void orderQueue();

		signals:
			/**
			* User tried to enqueue a torrent that has reached max share ratio. It's not possible.
			* Signal should be connected to SysTray slot which shows appropriate KPassivePopup info.
			* @param tc The torrent in question.
			*/
			void queuingNotPossible(kt::TorrentInterface* tc);
			
			/**
			 * Diskspace is running low.
			 * Signal should be connected to SysTray slot which shows appropriate KPassivePopup info. 
			 * @param tc The torrent in question.
			*/
			void lowDiskSpace(kt::TorrentInterface* tc, bool stopped);

		public slots:
			void torrentFinished(kt::TorrentInterface* tc);
			void torrentAdded(kt::TorrentInterface* tc, bool user, bool start_torrent);
			void torrentRemoved(kt::TorrentInterface* tc);
			void torrentStopped(kt::TorrentInterface* tc);
			void onLowDiskSpace(kt::TorrentInterface* tc, bool toStop);

		private:
			void enqueue(kt::TorrentInterface* tc);
			void dequeue(kt::TorrentInterface* tc);
			void startSafely(kt::TorrentInterface* tc);
			void stopSafely(kt::TorrentInterface* tc,bool user,WaitJob* wjob = 0);

			bt::QueuePtrList downloads;
			std::set<kt::TorrentInterface*> paused_torrents;

			int max_downloads;
			int max_seeds;


			bool paused_state;
			bool keep_seeding;
			bool exiting;
	};
}
#endif
