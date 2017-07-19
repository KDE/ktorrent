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

#include <QNetworkConfigurationManager>

#include <KLocalizedString>
#include <KMessageBox>

#include <util/log.h>
#include <util/error.h>
#include <util/sha1hash.h>
#include <util/waitjob.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>
#include <torrent/torrentcontrol.h>
#include <torrent/jobqueue.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/trackerslist.h>
#include <settings.h>
#include <algorithm>
#include <climits>


using namespace bt;

namespace kt
{

    QueueManager::QueueManager() : QObject()
    {
        max_downloads = 0;
        max_seeds = 0; //for testing. Needs to be added to Settings::

        keep_seeding = true; //test. Will be passed from Core
        suspended_state = false;
        exiting = false;
        ordering = false;

        QNetworkConfigurationManager* networkConfigurationManager = new QNetworkConfigurationManager(this);
        connect(networkConfigurationManager, &QNetworkConfigurationManager::onlineStateChanged, this, &QueueManager::onOnlineStateChanged);
    }


    QueueManager::~QueueManager()
    {
        qDeleteAll(downloads);
    }

    void QueueManager::append(bt::TorrentInterface* tc)
    {
        downloads.append(tc);
        connect(tc, &TorrentInterface::diskSpaceLow, this, &QueueManager::onLowDiskSpace);
        connect(tc, &TorrentInterface::torrentStopped, this, &QueueManager::torrentStopped);
        connect(tc, &TorrentInterface::updateQueue, this, &QueueManager::orderQueue);
    }

    void QueueManager::remove(bt::TorrentInterface* tc)
    {
        suspended_torrents.erase(tc);
        int index = downloads.indexOf(tc);
        if (index != -1)
            downloads.takeAt(index)->deleteLater();
    }

    void QueueManager::clear()
    {
        exiting = true;
        suspended_torrents.clear();
        qDeleteAll(downloads);
        downloads.clear();
    }

    TorrentStartResponse QueueManager::startInternal(bt::TorrentInterface* tc)
    {
        const TorrentStats& s = tc->getStats();

        if (!s.completed && !checkDiskSpace(tc, false))
            return bt::NOT_ENOUGH_DISKSPACE;
        else if (s.completed && !checkLimits(tc, false))
            return bt::MAX_SHARE_RATIO_REACHED;


        Out(SYS_GEN | LOG_NOTICE) << "Starting download " << s.torrent_name << endl;
        startSafely(tc);
        return START_OK;
    }

    bool QueueManager::checkLimits(TorrentInterface* tc, bool interactive)
    {
        QString msg;
        const TorrentStats& s = tc->getStats();
        bool max_ratio_reached = tc->overMaxRatio();
        bool max_seed_time_reached = tc->overMaxSeedTime();

        if (max_ratio_reached && max_seed_time_reached)
            msg = i18n("The torrent \"%1\" has reached its maximum share ratio and its maximum seed time. Ignore the limit and start seeding anyway?", s.torrent_name);
        else if (max_ratio_reached && !max_seed_time_reached)
            msg = i18n("The torrent \"%1\" has reached its maximum share ratio. Ignore the limit and start seeding anyway?", s.torrent_name);
        else if (max_seed_time_reached && !max_ratio_reached)
            msg = i18n("The torrent \"%1\" has reached its maximum seed time. Ignore the limit and start seeding anyway?", s.torrent_name);
        else
            return true;

        if (interactive && KMessageBox::questionYesNo(0, msg, i18n("Limits reached.")) == KMessageBox::Yes)
        {
            if (max_ratio_reached)
                tc->setMaxShareRatio(0.00f);
            if (max_seed_time_reached)
                tc->setMaxSeedTime(0.0f);
            return true;
        }

        return false;
    }

    bool QueueManager::checkDiskSpace(TorrentInterface* tc, bool interactive)
    {
        if (tc->checkDiskSpace(false))
            return true;

        //we're short!
        switch (Settings::startDownloadsOnLowDiskSpace())
        {
        case 0: //don't start!
            return false;
        case 1: //ask user
        {
            const TorrentStats& s = tc->getStats();
            QString msg = i18n(
                              "You don't have enough disk space to download this torrent. "
                              "Are you sure you want to continue?");

            QString caption = i18n("Insufficient disk space for %1", s.torrent_name);
            if (!interactive || KMessageBox::questionYesNo(0, msg, caption) == KMessageBox::No)
                return false;
            else
                break;
        }
        case 2: //force start
            break;
        }

        return true;
    }


    TorrentStartResponse QueueManager::start(bt::TorrentInterface* tc)
    {
        if (tc->getJobQueue()->runningJobs())
        {
            tc->setAllowedToStart(true);
            return BUSY_WITH_JOB;
        }

        const TorrentStats& s = tc->getStats();
        if (!s.completed && !checkDiskSpace(tc, true))
        {
            return bt::NOT_ENOUGH_DISKSPACE;
        }
        else if (s.completed && !checkLimits(tc, true))
        {
            return bt::MAX_SHARE_RATIO_REACHED;
        }

        if (!enabled())
        {
            return startInternal(tc);
        }
        else
        {
            tc->setAllowedToStart(true);
            orderQueue();
            return START_OK;
        }
    }

    void QueueManager::stop(bt::TorrentInterface* tc)
    {
        if (tc->getJobQueue()->runningJobs())
            return;

        const TorrentStats& s = tc->getStats();
        if (enabled())
            tc->setAllowedToStart(false);

        if (s.running)
            stopSafely(tc);
        else
            tc->setQueued(false);
    }

    void QueueManager::stop(QList<bt::TorrentInterface*> & todo)
    {
        ordering = true;
        for (bt::TorrentInterface* tc : qAsConst(todo))
        {
            stop(tc);
        }
        ordering = false;
        if (enabled())
            orderQueue();
    }

    void QueueManager::checkDiskSpace(QList<bt::TorrentInterface*> & todo)
    {
        // first see if we need to ask the user to start torrents when diskspace is low
        if (Settings::startDownloadsOnLowDiskSpace() == 2)
        {
            QStringList names;
            QList<bt::TorrentInterface*> tmp;
            for (bt::TorrentInterface* tc : qAsConst(todo))
            {
                const TorrentStats& s = tc->getStats();
                if (!s.completed && !tc->checkDiskSpace(false))
                {
                    names.append(s.torrent_name);
                    tmp.append(tc);
                }
            }

            if (tmp.count() > 0)
            {
                if (KMessageBox::questionYesNoList(0, i18n("Not enough disk space for the following torrents. Do you want to start them anyway?"), names) == KMessageBox::No)
                {
                    for (bt::TorrentInterface* tc : qAsConst(tmp))
                        todo.removeAll(tc);
                }
            }
        }
        // if the policy is to not start, remove torrents from todo list if diskspace is low
        else if (Settings::startDownloadsOnLowDiskSpace() == 0)
        {
            QList<bt::TorrentInterface*>::iterator i = todo.begin();
            while (i != todo.end())
            {
                bt::TorrentInterface* tc = *i;
                const TorrentStats& s = tc->getStats();
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
        for (bt::TorrentInterface* tc : qAsConst(todo))
        {
            const TorrentStats& s = tc->getStats();
            if (s.completed && tc->overMaxSeedTime())
            {
                names.append(s.torrent_name);
                tmp.append(tc);
            }
        }

        if (tmp.count() > 0)
        {
            if (KMessageBox::questionYesNoList(0, i18n("The following torrents have reached their maximum seed time. Do you want to start them anyway?"), names) == KMessageBox::No)
            {
                for (bt::TorrentInterface* tc : qAsConst(tmp))
                    todo.removeAll(tc);
            }
            else
            {
                for (bt::TorrentInterface* tc : qAsConst(tmp))
                    tc->setMaxSeedTime(0.0f);
            }
        }
    }

    void QueueManager::checkMaxRatio(QList<bt::TorrentInterface*> & todo)
    {
        QStringList names;
        QList<bt::TorrentInterface*> tmp;
        for (bt::TorrentInterface* tc : qAsConst(todo))
        {
            const TorrentStats& s = tc->getStats();
            if (s.completed && tc->overMaxRatio())
            {
                names.append(s.torrent_name);
                tmp.append(tc);
            }
        }

        if (tmp.count() > 0)
        {
            if (KMessageBox::questionYesNoList(0, i18n("The following torrents have reached their maximum share ratio. Do you want to start them anyway?"), names) == KMessageBox::No)
            {
                for (bt::TorrentInterface* tc : qAsConst(tmp))
                    todo.removeAll(tc);
            }
            else
            {
                for (bt::TorrentInterface* tc : qAsConst(tmp))
                    tc->setMaxShareRatio(0.0f);
            }
        }
    }

    void QueueManager::start(QList<bt::TorrentInterface*> & todo)
    {
        if (todo.count() == 0)
            return;

        // check diskspace stuff
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
        for (bt::TorrentInterface* tc : qAsConst(todo))
        {
            const TorrentStats& s = tc->getStats();
            if (s.running)
                continue;

            if (tc->getJobQueue()->runningJobs())
                continue;

            if (enabled())
                tc->setAllowedToStart(true);
            else
                startSafely(tc);
        }

        if (enabled())
            orderQueue();
    }

    void QueueManager::startAll()
    {
        if (enabled())
        {
            for (bt::TorrentInterface* tc : qAsConst(downloads))
                tc->setAllowedToStart(true);

            orderQueue();
        }
        else
        {
            // first get the list of torrents which need to be started
            QList<bt::TorrentInterface*> todo;
            for (bt::TorrentInterface* tc : qAsConst(downloads))
            {
                const TorrentStats& s = tc->getStats();
                if (s.running)
                    continue;

                if (tc->getJobQueue()->runningJobs())
                    continue;

                todo.append(tc);
            }

            start(todo);
        }
    }

    void QueueManager::stopAll()
    {
        stop(downloads);
    }


    void QueueManager::startAutoStartTorrents()
    {
        if (enabled() || suspended_state)
            return;

        // first get the list of torrents which need to be started
        QList<bt::TorrentInterface*> todo;
        for (bt::TorrentInterface* tc : qAsConst(downloads))
        {
            const TorrentStats& s = tc->getStats();
            if (s.running || tc->getJobQueue()->runningJobs() || !s.autostart)
                continue;

            todo.append(tc);
        }

        start(todo);
    }


    void QueueManager::onExit(WaitJob* wjob)
    {
        exiting = true;
        QList<bt::TorrentInterface*>::iterator i = downloads.begin();
        while (i != downloads.end())
        {
            bt::TorrentInterface* tc = *i;
            if (tc->getStats().running)
            {
                stopSafely(tc, wjob);
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
        QList<TorrentInterface*>::const_iterator i = downloads.constBegin();
        while (i != downloads.constEnd())
        {
            const TorrentInterface* tc = *i;
            const TorrentStats& s = tc->getStats();

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

    bt::TorrentInterface* QueueManager::getTorrent(bt::Uint32 idx)
    {
        if (idx >= (Uint32)downloads.count())
            return 0;
        else
            return downloads[idx];
    }

    QList<bt::TorrentInterface*>::iterator QueueManager::begin()
    {
        return downloads.begin();
    }

    QList<bt::TorrentInterface*>::iterator QueueManager::end()
    {
        return downloads.end();
    }

    void QueueManager::setMaxDownloads(int m)
    {
        max_downloads = m;
    }

    void QueueManager::onLowDiskSpace(bt::TorrentInterface* tc, bool toStop)
    {
        if (toStop)
        {
            stopSafely(tc);
            if (enabled())
            {
                tc->setAllowedToStart(false);
                orderQueue();
            }
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

    bool QueueManager::alreadyLoaded(const bt::SHA1Hash& ih) const
    {
        for (const bt::TorrentInterface* tor : qAsConst(downloads))
        {
            if (tor->getInfoHash() == ih)
                return true;
        }
        return false;
    }

    void QueueManager::mergeAnnounceList(const bt::SHA1Hash& ih, const TrackerTier* trk)
    {
        for (bt::TorrentInterface* tor : qAsConst(downloads))
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
        if (ordering || !downloads.count() || exiting)
            return;

        emit orderingQueue();

        downloads.sort(); // sort downloads, even when suspended so that the QM widget is updated
        if (Settings::manuallyControlTorrents() || suspended_state)
        {
            emit queueOrdered();
            return;
        }

        RecursiveEntryGuard guard(&ordering); // make sure that recursive entering of this function is not possible

        QueuePtrList download_queue;
        QueuePtrList seed_queue;

        for (TorrentInterface* tc : qAsConst(downloads))
        {
            const TorrentStats& s = tc->getStats();
            if (s.running || (tc->isAllowedToStart() && !s.stopped_by_error && !tc->getJobQueue()->runningJobs()))
            {
                if (s.completed)
                {
                    if (s.running || (!tc->overMaxRatio() && !tc->overMaxSeedTime()))
                        seed_queue.append(tc);
                }
                else
                    download_queue.append(tc);
            }
        }

        int num_running = 0;
        for (bt::TorrentInterface* tc : qAsConst(download_queue))
        {
            const TorrentStats& s = tc->getStats();

            if (num_running < max_downloads || max_downloads == 0)
            {
                if (!s.running)
                {
                    Out(SYS_GEN | LOG_DEBUG) << "QM Starting: " << s.torrent_name << endl;
                    if (startInternal(tc) == bt::START_OK)
                        num_running++;
                }
                else
                    num_running++;
            }
            else
            {
                if (s.running)
                {
                    Out(SYS_GEN | LOG_DEBUG) << "QM Stopping: " << s.torrent_name << endl;
                    stopSafely(tc);
                }
                tc->setQueued(true);
            }
        }

        num_running = 0;
        for (bt::TorrentInterface* tc : qAsConst(seed_queue))
        {
            const TorrentStats& s = tc->getStats();
            if (num_running < max_seeds || max_seeds == 0)
            {
                if (!s.running)
                {
                    Out(SYS_GEN | LOG_DEBUG) << "QM Starting: " << s.torrent_name << endl;
                    if (startInternal(tc) == bt::START_OK)
                        num_running++;
                }
                else
                    num_running++;
            }
            else
            {
                if (s.running)
                {
                    Out(SYS_GEN | LOG_DEBUG) << "QM Stopping: " << s.torrent_name << endl;
                    stopSafely(tc);
                }
                tc->setQueued(true);
            }
        }

        emit queueOrdered();
    }

    void QueueManager::torrentFinished(bt::TorrentInterface* tc)
    {
        if (!keep_seeding)
        {
            if (enabled())
                tc->setAllowedToStart(false);

            stopSafely(tc);
        }

        orderQueue();
    }

    void QueueManager::torrentAdded(bt::TorrentInterface* tc, bool start_torrent)
    {
        if (enabled())
        {
            // new torrents have the lowest priority
            // so everybody else gets a higher priority
            for (TorrentInterface* otc : qAsConst(downloads))
            {
                int p = otc->getPriority();
                otc->setPriority(p + 1);
            }
            tc->setAllowedToStart(start_torrent);
            tc->setPriority(0);
            rearrangeQueue();
            orderQueue();
        }
        else
        {
            if (start_torrent)
                start(tc);
        }
    }

    void QueueManager::torrentRemoved(bt::TorrentInterface* tc)
    {
        remove(tc);
        rearrangeQueue();
        orderQueue();
    }

    void QueueManager::torrentsRemoved(QList<bt::TorrentInterface*>& tors)
    {
        for (bt::TorrentInterface* tc : qAsConst(tors))
            remove(tc);
        rearrangeQueue();
        orderQueue();
    }


    void QueueManager::setSuspendedState(bool suspend)
    {
        if (suspended_state == suspend)
            return;

        suspended_state = suspend;
        if (!suspend)
        {
            UpdateCurrentTime();
            std::set<bt::TorrentInterface*>::iterator it = suspended_torrents.begin();
            while (it != suspended_torrents.end())
            {
                TorrentInterface* tc = *it;
                startSafely(tc);
                it++;
            }

            suspended_torrents.clear();
            orderQueue();
        }
        else
        {
            for (TorrentInterface* tc : qAsConst(downloads))
            {
                const TorrentStats& s = tc->getStats();
                if (s.running)
                {
                    suspended_torrents.insert(tc);
                    stopSafely(tc);
                }
            }
        }
        emit suspendStateChanged(suspended_state);
    }

    void QueueManager::rearrangeQueue()
    {
        downloads.sort();
        reindexQueue();
    }

    void QueueManager::startSafely(bt::TorrentInterface* tc)
    {
        try
        {
            tc->start();
        }
        catch (bt::Error& err)
        {
            const TorrentStats& s = tc->getStats();
            QString msg =
                i18n("Error starting torrent %1: %2",
                     s.torrent_name, err.toString());
            KMessageBox::error(0, msg, i18n("Error"));
        }
    }

    void QueueManager::stopSafely(bt::TorrentInterface* tc, WaitJob* wjob)
    {
        try
        {
            tc->stop(wjob);
        }
        catch (bt::Error& err)
        {
            const TorrentStats& s = tc->getStats();
            QString msg =
                i18n("Error stopping torrent %1: %2",
                     s.torrent_name, err.toString());
            KMessageBox::error(0, msg, i18n("Error"));
        }
    }

    void QueueManager::torrentStopped(bt::TorrentInterface*)
    {
        orderQueue();
    }

    static bool IsStalled(bt::TorrentInterface* tc, bt::TimeStamp now, bt::Uint32 min_stall_time)
    {
        bt::Int64 stalled_time = 0;
        if (tc->getStats().completed)
            stalled_time = (now - tc->getStats().last_upload_activity_time) / 1000;
        else
            stalled_time = (now - tc->getStats().last_download_activity_time) / 1000;

        return stalled_time > min_stall_time * 60 && tc->getStats().running;
    }

    void QueueManager::checkStalledTorrents(bt::TimeStamp now, bt::Uint32 min_stall_time)
    {
        if (!enabled())
            return;

        QueuePtrList newlist;
        QueuePtrList stalled;
        bool can_decrease = false;

        // find all stalled ones
        for (bt::TorrentInterface* tc : qAsConst(downloads))
        {
            if (IsStalled(tc, now, min_stall_time))
            {
                stalled.append(tc);
            }
            else
            {
                // decreasing makes only sense if there are QM torrents after the stalled ones
                can_decrease = stalled.count() > 0;
                newlist.append(tc);
            }
        }

        if (stalled.count() == 0 || stalled.count() == downloads.count() || !can_decrease)
            return;

        for (bt::TorrentInterface* tc : qAsConst(stalled))
            Out(SYS_GEN | LOG_NOTICE) << "The torrent " << tc->getStats().torrent_name << " has stalled longer than " << min_stall_time << " minutes, decreasing its priority" << endl;

        downloads.clear();
        downloads += newlist;
        downloads += stalled;
        // redo priorities and then order the queue
        int prio = downloads.count();
        for (bt::TorrentInterface* tc : qAsConst(downloads))
        {
            tc->setPriority(prio--);
        }
        orderQueue();
    }

    void QueueManager::onOnlineStateChanged(bool isOnline)
    {
        if (isOnline)
        {
            Out(SYS_GEN | LOG_IMPORTANT) << "Network is up" << endl;
            // if the network has gone down, longer then 2 minutes
            // all the connections are probably stale, so tell all
            // running torrents, that they need to reannounce and kill stale peers
            if (network_down_time.isValid() && network_down_time.secsTo(QDateTime::currentDateTime()) > 120)
            {
                for (bt::TorrentInterface* tc : qAsConst(downloads))
                {
                    if (tc->getStats().running)
                        tc->networkUp();
                }
            }

            network_down_time = QDateTime();
        }
        else
        {
            Out(SYS_GEN | LOG_IMPORTANT) << "Network is down" << endl;
            network_down_time = QDateTime::currentDateTime();
        }
    }

    void QueueManager::reindexQueue()
    {
        int prio = downloads.count();
        // make sure everybody has an unique priority
        for (bt::TorrentInterface* tc : qAsConst(downloads))
        {
            tc->setPriority(prio--);
        }
    }

    void QueueManager::loadState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("QueueManager");
        suspended_state = g.readEntry("suspended", false);

        if (suspended_state)
        {
            QStringList info_hash_list = g.readEntry("suspended_torrents", QStringList());
            for (bt::TorrentInterface* t : qAsConst(downloads))
            {
                if (info_hash_list.contains(t->getInfoHash().toString()))
                    suspended_torrents.insert(t);
            }
        }
    }

    void QueueManager::saveState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("QueueManager");
        g.writeEntry("suspended", suspended_state);

        if (suspended_state)
        {
            QStringList info_hash_list;
            for (bt::TorrentInterface* t : qAsConst(suspended_torrents))
            {
                info_hash_list << t->getInfoHash().toString();
            }
            g.writeEntry("suspended_torrents", info_hash_list);
        }
    }

    bool QueueManager::checkFileConflicts(TorrentInterface* tc, QStringList& conflicting) const
    {
        conflicting.clear();

        // First get a set off all files of tc
        QSet<QString> files;
        if (tc->getStats().multi_file_torrent)
        {
            for (bt::Uint32 i = 0; i < tc->getNumFiles(); i++)
                files.insert(tc->getTorrentFile(i).getPathOnDisk());
        }
        else
            files.insert(tc->getStats().output_path);

        for (bt::TorrentInterface* t : qAsConst(downloads))
        {
            if (t == tc)
                continue;

            if (t->getStats().multi_file_torrent)
            {
                for (bt::Uint32 i = 0; i < t->getNumFiles(); i++)
                {
                    if (files.contains(t->getTorrentFile(i).getPathOnDisk()))
                    {
                        conflicting.append(t->getDisplayName());
                        break;
                    }
                }
            }
            else
            {
                if (files.contains(t->getStats().output_path))
                    conflicting.append(t->getDisplayName());
            }
        }

        return !conflicting.isEmpty();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////


    QueuePtrList::QueuePtrList() : QList<bt::TorrentInterface*>()
    {}

    QueuePtrList::~QueuePtrList()
    {}

    void QueuePtrList::sort()
    {
        std::sort(begin(), end(), QueuePtrList::biggerThan);
    }

    bool QueuePtrList::biggerThan(bt::TorrentInterface* tc1, bt::TorrentInterface* tc2)
    {
        return tc1->getPriority() > tc2->getPriority();
    }

}

