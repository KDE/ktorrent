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

#include <QObject>
#include <KSharedConfig>

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

    class KTCORE_EXPORT QueuePtrList : public QList<bt::TorrentInterface*>
    {
    public:
        QueuePtrList();
        ~QueuePtrList();

        /**
         * Sort based upon priority
         */
        void sort();

    protected:
        static bool biggerThan(bt::TorrentInterface* tc1, bt::TorrentInterface* tc2);
    };

    /**
     * @author Ivan Vasic
     * @brief This class contains list of all TorrentControls and is responsible for starting/stopping them
     */
    class KTCORE_EXPORT QueueManager : public QObject, public bt::QueueManagerInterface
    {
        Q_OBJECT

    public:
        QueueManager();
        ~QueueManager();

        void append(bt::TorrentInterface* tc);
        void remove(bt::TorrentInterface* tc);
        void clear();

        /**
            Save the state of the QueueManager
            @param cfg The config
        */
        void saveState(KSharedConfigPtr cfg);

        /**
            Load the state of the QueueManager
            @param cfg The config
        */
        void loadState(KSharedConfigPtr cfg);

        /**
         * Check if we need to decrease the priority of stalled torrents
         * @param min_stall_time Stall time in minutes
         * @param now The current time
         */
        void checkStalledTorrents(bt::TimeStamp now, bt::Uint32 min_stall_time);

        /**
         * Start a torrent
         * @param tc The torrent
         * @return What happened
         */
        bt::TorrentStartResponse start(bt::TorrentInterface* tc);

        /**
         * Stop a torrent
         * @param tc The torrent
         */
        void stop(bt::TorrentInterface* tc);

        /**
         * Start a list of torrents.
         * @param todo The list of torrents
         */
        void start(QList<bt::TorrentInterface*> & todo);

        /**
         * Stop a list of torrents
         * @param todo The list of torrents
         */
        void stop(QList<bt::TorrentInterface*> & todo);

        /// Stop all torrents
        void stopAll();

        /// Start all torrents
        void startAll();

        /**
         * Stop all running torrents
         * @param wjob WaitJob which waits for stopped events to reach the tracker
         */
        void onExit(bt::WaitJob* wjob);

        /// Get the number of torrents
        int count() { return downloads.count(); }

        /// Get the number of downloads
        int countDownloads();

        /// Get the number of seeds
        int countSeeds();

        enum Flags
        {
            SEEDS = 1,
            DOWNLOADS = 2,
            ALL = 3
        };

        /**
         * Get the number of running torrents
         * @param flags Which torrents to choose
         */
        int getNumRunning(Flags flags = ALL);

        /**
         * Start the next torrent.
         */
        void startNext();

        /**
            If the QM is disabled this function needs to be called to start
            all the torrents which were running at the time of the previous exit.
        */
        void startAutoStartTorrents();

        typedef QList<bt::TorrentInterface*>::iterator iterator;

        iterator begin();
        iterator end();

        /**
         * Get the torrent at index idx in the list.
         * @param idx Index of the torrent
         * @return The torrent or 0 if the index is out of bounds
         */
        const bt::TorrentInterface* getTorrent(bt::Uint32 idx) const;

        /**
         * Get the torrent at index idx in the list.
         * @param idx Index of the torrent
         * @return The torrent or 0 if the index is out of bounds
         */
        bt::TorrentInterface* getTorrent(bt::Uint32 idx);

        /**
         * See if we already loaded a torrent.
         * @param ih The info hash of a torrent
         * @return true if we do, false if we don't
         */
        bool alreadyLoaded(const bt::SHA1Hash& ih) const;


        /**
         * Merge announce lists to a torrent
         * @param ih The info_hash of the torrent to merge to
         * @param trk First tier of trackers
         */
        void mergeAnnounceList(const bt::SHA1Hash& ih, const bt::TrackerTier* trk);

        /**
         * Set the maximum number of downloads
         * @param m Max downloads
         */
        void setMaxDownloads(int m);

        /**
         * Set the maximum number of seeds
         * @param m Max seeds
         */
        void setMaxSeeds(int m);

        /**
         * Enable or disable keep seeding (after a torrent has finished)
         * @param ks Keep seeding
         */
        void setKeepSeeding(bool ks);

        /**
         * Sets global suspended state for QueueManager and stopps all running torrents.
         * No torrents will be automatically started/stopped with QM.
         */
        void setSuspendedState(bool suspend);

        /// Get the suspended state
        bool getSuspendedState() const {return suspended_state;}

        /**
         * Reindex the queue priorities.
         */
        void reindexQueue();

        /**
         * Check if a torrent has file conflicts with other torrents.
         * If conflicting are found, a list of names of the conflicting torrents is filled in.
         * @param tc The torrent
         * @param conflicting List of conflicting torrents
         */
        bool checkFileConflicts(bt::TorrentInterface* tc, QStringList& conflicting) const;

        /**
         * Places all torrents from downloads in the right order in queue.
         * Use this when torrent priorities get changed
         */
        void orderQueue();

    Q_SIGNALS:
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

        /// Emitted before the queue is reordered
        void orderingQueue();

        /**
        * Emitted when the QM has reordered it's queue
        */
        void queueOrdered();

        /**
         * Emitted when the suspended state changes.
         * @param suspended The suspended state
         */
        void suspendStateChanged(bool suspended);

    public Q_SLOTS:
        void torrentFinished(bt::TorrentInterface* tc);
        void torrentAdded(bt::TorrentInterface* tc, bool start_torrent);
        void torrentRemoved(bt::TorrentInterface* tc);
        void torrentsRemoved(QList<bt::TorrentInterface*> & tors);
        void torrentStopped(bt::TorrentInterface* tc);
        void onLowDiskSpace(bt::TorrentInterface* tc, bool toStop);
        void onOnlineStateChanged(bool);

    private:
        void startSafely(bt::TorrentInterface* tc);
        void stopSafely(bt::TorrentInterface* tc, bt::WaitJob* wjob = 0);
        void checkDiskSpace(QList<bt::TorrentInterface*> & todo);
        void checkMaxSeedTime(QList<bt::TorrentInterface*> & todo);
        void checkMaxRatio(QList<bt::TorrentInterface*> & todo);
        void rearrangeQueue();
        bt::TorrentStartResponse startInternal(bt::TorrentInterface* tc);
        bool checkLimits(bt::TorrentInterface* tc, bool interactive);
        bool checkDiskSpace(bt::TorrentInterface* tc, bool interactive);

    private:
        QueuePtrList downloads;
        std::set<bt::TorrentInterface*> suspended_torrents;
        int max_downloads;
        int max_seeds;
        bool suspended_state;
        bool keep_seeding;
        bool exiting;
        bool ordering;
        QDateTime network_down_time;
    };
}
#endif
