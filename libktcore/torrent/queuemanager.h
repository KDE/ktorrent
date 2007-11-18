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
#ifndef KTQUEUEMANAGER_H
#define KTQUEUEMANAGER_H

#include <set>
#include <qobject.h>
#include <qlinkedlist.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/queuemanagerinterface.h>
#include <ktcore_export.h>

namespace bt
{
	class SHA1Hash;
	struct TrackerTier;
	class WaitJob;
}

namespace kt
{

	class QueuePtrList : public QList<bt::TorrentInterface *>
	{
		public:
			QueuePtrList();
			virtual ~QueuePtrList();

		protected:
			int compareItems(bt::TorrentInterface* tc1, bt::TorrentInterface* tc2);
	};

	/**
	 * @author Ivan Vasic
	 * @brief This class contains list of all TorrentControls and is responsible for starting/stopping them
	 */
	class KTCORE_EXPORT QueueManager : public QObject,public bt::QueueManagerInterface
	{
		Q_OBJECT
				
		public:
			QueueManager();
			virtual ~QueueManager();

			void append(bt::TorrentInterface* tc);
			void remove(bt::TorrentInterface* tc);
			void clear();
			
			bt::TorrentStartResponse start(bt::TorrentInterface* tc, bool user = true);
			void stop(bt::TorrentInterface* tc, bool user = false);

			void stopall(int type);
			void startall(int type);
			
			/**
			 * Stop all running torrents
			 * @param wjob WaitJob which waits for stopped events to reach the tracker
			 */
			void onExit(bt::WaitJob* wjob);

			/**
			 * Enqueue/Dequeue function. Places a torrent in queue. 
			 * If the torrent is already in queue this will remove it from queue.
			 * @param tc TorrentControl pointer.
			 */
			void queue(bt::TorrentInterface* tc);

			int count() { return downloads.count(); }
			int countDownloads();
			int countSeeds();

			int getNumRunning(bool onlyDownload = false, bool onlySeed = false);
			int getNumRunning(bool userControlled, bool onlyDownloads, bool onlySeeds);

			void startNext();

			QList<bt::TorrentInterface *>::iterator begin();
			QList<bt::TorrentInterface *>::iterator end();

			/**
			 * See if we already loaded a torrent.
			 * @param ih The info hash of a torrent
			 * @return true if we do, false if we don't
			 */
			bool allreadyLoaded(const bt::SHA1Hash & ih) const;


			/**
			 * Merge announce lists to a torrent
			 * @param ih The info_hash of the torrent to merge to
			 * @param trk First tier of trackers
			 */
			void mergeAnnounceList(const bt::SHA1Hash & ih,const bt::TrackerTier* trk);

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
			void queuingNotPossible(bt::TorrentInterface* tc);

			/**
 			 * Diskspace is running low.
 			 * Signal should be connected to SysTray slot which shows appropriate KPassivePopup info. 
 			 * @param tc The torrent in question.
 			*/
 			void lowDiskSpace(bt::TorrentInterface* tc, bool stopped);

		public slots:
			void torrentFinished(bt::TorrentInterface* tc);
			void torrentAdded(bt::TorrentInterface* tc,bool user, bool start_torrent);
			void torrentRemoved(bt::TorrentInterface* tc);
			void torrentStopped(bt::TorrentInterface* tc);
			void onLowDiskSpace(bt::TorrentInterface* tc, bool toStop);

		private:
			void enqueue(bt::TorrentInterface* tc);
			void dequeue(bt::TorrentInterface* tc);
			void startSafely(bt::TorrentInterface* tc);
			void stopSafely(bt::TorrentInterface* tc,bool user,bt::WaitJob* wjob = 0);

			QueuePtrList downloads;
			std::set<bt::TorrentInterface*> paused_torrents;

			int max_downloads;
			int max_seeds;


			bool paused_state;
			bool keep_seeding;
			bool exiting;
	};
}
#endif
