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
#ifndef QUEUEMANAGER_H
#define QUEUEMANAGER_H

#include <qobject.h>
#include <qptrlist.h>

#include <interfaces/torrentinterface.h>

namespace bt
{
	class SHA1Hash;
	
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
		public:
			QueueManager();
			~QueueManager();
			
			void append(kt::TorrentInterface* tc);
			void remove(kt::TorrentInterface* tc);
			void clear();
			
			void start(kt::TorrentInterface* tc);
			void stop(kt::TorrentInterface* tc, bool user = false);
			
			void stopall();
			void startall();
			
			int count() { return downloads.count(); }
			int countDownloads();
			int countSeeds();
			
			int getNumRunning(bool onlyDownload = false, bool onlySeed = false);
			
			void startNext();
			
			QPtrList<kt::TorrentInterface>::iterator begin();
			QPtrList<kt::TorrentInterface>::iterator end();
			
			/**
			 * See if we already loaded a torrent.
			 * @param ih The info hash of a torrent
			 * @return true if we do, false if we don't
			 */
			bool allreadyLoaded(const SHA1Hash & ih) const;
			
			void setMaxDownloads(int m);
			void setMaxSeeds(int m);
			
			void setKeepSeeding(bool ks);
			
			/**
			 * Places all torrents from downloads in the right order in queue.
			 * Use this when torrent priorities get changed
			 */
			void orderQueue();
		
		public slots:
			void torrentFinished(kt::TorrentInterface* tc);
			void torrentAdded(kt::TorrentInterface* tc);
			void torrentRemoved(kt::TorrentInterface* tc);
			
		private:
			bt::QueuePtrList downloads;
			
			int max_downloads;
			int max_seeds;
			
			bool keep_seeding;
	};
}
#endif
