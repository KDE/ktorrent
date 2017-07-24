/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson and Ivan Vasic                    *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "core.h"

#include <QDir>
#include <QNetworkInterface>
#include <QProgressBar>

#include <KLocalizedString>
#include <KIO/Job>
#include <KIO/CopyJob>
#include <KMessageBox>
#include <KStandardGuiItem>

#include <dbus/dbus.h>
#include <interfaces/guiinterface.h>
#include <interfaces/functions.h>
#include <interfaces/torrentfileinterface.h>
#include <torrent/magnetmanager.h>
#include <torrent/queuemanager.h>
#include <torrent/torrentcontrol.h>
#include <torrent/torrentcreator.h>
#include <torrent/server.h>
#include <peer/authenticationmonitor.h>
#include <util/log.h>
#include <util/error.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <util/waitjob.h>
#include <bcodec/bencoder.h>
#include <bcodec/bnode.h>
#include <plugin/pluginmanager.h>
#include <groups/groupmanager.h>
#include <groups/group.h>
#include <dht/dht.h>
#include <dht/dhtbase.h>
#include <utp/utpserver.h>
#include <net/socketmonitor.h>
#include <torrent/jobqueue.h>
#include "settings.h"
#include "dialogs/fileselectdlg.h"
#include "dialogs/missingfilesdlg.h"
#include "gui.h"
#include "powermanagementinhibit_interface.h"


using namespace bt;

namespace kt
{
    const Uint32 CORE_UPDATE_INTERVAL = 250;

    Core::Core(kt::GUI* gui)
        : gui(gui),
          keep_seeding(true),
          sleep_suppression_cookie(0),
          exiting(false),
          reordering_queue(false)
    {
        UpdateCurrentTime();
        qman = new QueueManager();
        connect(qman, &kt::QueueManager::lowDiskSpace, this, &Core::onLowDiskSpace);
        connect(qman, &kt::QueueManager::queuingNotPossible, this, &Core::enqueueTorrentOverMaxRatio);
        connect(qman, &kt::QueueManager::lowDiskSpace, this, &Core::onLowDiskSpace);
        connect(qman, &kt::QueueManager::orderingQueue, this, &Core::beforeQueueReorder);
        connect(qman, &kt::QueueManager::queueOrdered, this, &Core::afterQueueReorder);

        data_dir = Settings::tempDir();
        bool dd_not_exist = !bt::Exists(data_dir);
        if (data_dir.isEmpty() || dd_not_exist)
        {
            data_dir = kt::DataDir();
            if (dd_not_exist)
            {
                Settings::setTempDir(data_dir);
                Settings::self()->save();
            }
        }

        removed_bytes_up = removed_bytes_down = 0;

        if (!data_dir.endsWith(bt::DirSeparator()))
            data_dir += bt::DirSeparator();

        connect(&update_timer, &QTimer::timeout, this, &Core::update);

        // Make sure network interface is set properly before server is initialized
        if (!Settings::networkInterface().isEmpty())
        {
        //    QList<QNetworkInterface> iface_list = QNetworkInterface::allInterfaces();
            QString iface = Settings::networkInterface();
            SetNetworkInterface(iface);
        }


        startServers();

        mman = new kt::MagnetManager(this);
        pman = new kt::PluginManager(this, gui);
        gman = new kt::GroupManager();
        applySettings();
        gman->loadGroups();
        connect(gman, &kt::GroupManager::customGroupChanged, this, &Core::customGroupChanged);

        qRegisterMetaType<bt::MagnetLink>("bt::MagnetLink");
        qRegisterMetaType<kt::MagnetLinkLoadOptions>("kt::MagnetLinkLoadOptions");
        connect(mman, &kt::MagnetManager::metadataDownloaded, this, &Core::onMetadataDownloaded, Qt::QueuedConnection);

        mman->loadMagnets(kt::DataDir() + QLatin1String("magnets"));

        connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &Core::onExit);
    }

    Core::~Core()
    {
        delete qman;
        delete pman;
        delete gman;
    }


    void Core::startServers()
    {
        Uint16 port = Settings::port();
        if (port == 0)
        {
            port = 6881;
            Settings::setPort(6881);
        }

        if (Settings::utpEnabled())
        {
            startUTPServer(port);
            if (!Settings::onlyUseUtp())
                startTCPServer(port);
        }
        else
        {
            startTCPServer(port);
        }
        ServerInterface::setPort(port);
    }

    void Core::startTCPServer(bt::Uint16 port)
    {
        if (Globals::instance().initTCPServer(port))
        {
            Out(SYS_GEN | LOG_NOTICE) << "Bound to TCP port " << port << endl;
        }
        else
        {
            gui->errorMsg(i18n("KTorrent is unable to accept connections because the TCP port %1 is "
                               "already in use by another program.", port));
            Out(SYS_GEN | LOG_IMPORTANT) << "Cannot find free TCP port" << endl;
        }
    }

    void Core::startUTPServer(bt::Uint16 port)
    {
        if (Globals::instance().initUTPServer(port))
        {
            Out(SYS_GEN | LOG_NOTICE) << "Bound to UDP port " << port << endl;
        }
        else
        {
            gui->errorMsg(i18n("KTorrent is unable to accept connections because the UDP port %1 is "
                               "already in use by another program.", port));
            Out(SYS_GEN | LOG_IMPORTANT) << "Cannot find free UDP port" << endl;
        }
    }



    void Core::applySettings()
    {
        bt::Uint16 port = Settings::port();
        bt::Uint16 current_port = ServerInterface::getPort();

        bool utp_enabled = Settings::utpEnabled();
        bool tcp_enabled = utp_enabled && Settings::onlyUseUtp() ? false : true;

        bt::Globals& globals = bt::Globals::instance();

        if (globals.isTCPEnabled() && !tcp_enabled)
            globals.shutdownTCPServer();
        else if (!globals.isTCPEnabled() && tcp_enabled)
            startTCPServer(port);
        else if (tcp_enabled && port != current_port)
            globals.getTCPServer().changePort(port);

        if (globals.isUTPEnabled() && !utp_enabled)
            globals.shutdownUTPServer();
        else if (!globals.isUTPEnabled() && utp_enabled)
            startUTPServer(port);
        else if (utp_enabled && port != current_port)
            globals.getUTPServer().changePort(port);

        if (utp_enabled)
            globals.getUTPServer().setTOS(Settings::dscp() << 2);

        ServerInterface::setPort(port);
        ServerInterface::setUtpEnabled(utp_enabled, Settings::onlyUseUtp());
        ServerInterface::setPrimaryTransportProtocol((bt::TransportProtocol)Settings::primaryTransportProtocol());
        ApplySettings();
        setMaxDownloads(Settings::maxDownloads());
        setMaxSeeds(Settings::maxSeeds());
        setKeepSeeding(Settings::keepSeeding());

        QString tmp = Settings::tempDir();
        if (tmp.isEmpty())
            tmp = kt::DataDir();

        changeDataDir(tmp);
        //update QM
        getQueueManager()->orderQueue();

        mman->setUseSlotTimer(Settings::requeueMagnets());
        mman->setTimerDuration(Settings::requeueMagnetsTime());
        mman->setDownloadingSlots(Settings::numMagnetDownloadingSlots());

        settingsChanged();
    }

    void Core::loadPlugins()
    {
        pman->loadPluginList();
    }

    bool Core::init(TorrentControl* tc, const QString& group, const QString& location, bool silently)
    {
        bool start_torrent = false;
        bool skip_check = false;
        QString selected_group = group;

        if (Settings::maxRatio() > 0)
            tc->setMaxShareRatio(Settings::maxRatio());
        if (Settings::maxSeedTime() > 0)
            tc->setMaxSeedTime(Settings::maxSeedTime());

        if (Settings::useCompletedDir() && (silently || Settings::openAllTorrentsSilently()))
            tc->setMoveWhenCompletedDir(Settings::completedDir());

        if (qman->alreadyLoaded(tc->getInfoHash()))
        {
            Out(SYS_GEN | LOG_IMPORTANT) << "Torrent " << tc->getDisplayName() << " already loaded" << endl;
            return false;
        }

        if (!silently && !Settings::openAllTorrentsSilently())
        {
            FileSelectDlg dlg(qman, gman, group, gui->getMainWindow());
            dlg.loadState(KSharedConfig::openConfig());
            bool ret = dlg.execute(tc, &start_torrent, &skip_check, location) == QDialog::Accepted;
            dlg.saveState(KSharedConfig::openConfig());

            if (!ret)
                return false;
            else
                selected_group = dlg.selectedGroup();
        }
        else
            start_torrent = true;


        QStringList conflicting;
        if (qman->checkFileConflicts(tc, conflicting))
        {
            Out(SYS_GEN | LOG_IMPORTANT) << "Torrent " << tc->getDisplayName() << " conflicts with the following torrents: " << endl;
            Out(SYS_GEN | LOG_IMPORTANT) << conflicting.join(QStringLiteral(", ")) << endl;
            if (!silently)
            {
                QString err = i18n("Opening the torrent <b>%1</b>, "
                                   "would share one or more files with the following torrents. "
                                   "Torrents are not allowed to write to the same files. ", tc->getDisplayName());
                KMessageBox::errorList(gui, err, conflicting);
            }

            return false;
        }

        try
        {
            tc->createFiles();
        }
        catch (bt::Error& err)
        {
            if (!silently)
                gui->errorMsg(err.toString());
            Out(SYS_GEN | LOG_IMPORTANT) << err.toString() << endl;
            return false;
        }

        if (tc->hasExistingFiles())
        {
            if (!skip_check)
                doDataCheck(tc, true);
            else
                tc->markExistingFilesAsDownloaded();
        }


        tc->setPreallocateDiskSpace(true);
        connectSignals(tc);
        qman->append(tc);
        qman->torrentAdded(tc, start_torrent);

        //now copy torrent file to user specified dir if needed
        if (Settings::useTorrentCopyDir())
        {
            QString torFile = tc->getTorDir();
            if (!torFile.endsWith(bt::DirSeparator()))
                torFile += bt::DirSeparator();

            torFile += QLatin1String("torrent");
            QString destination = Settings::torrentCopyDir();
            if (!destination.endsWith(bt::DirSeparator()))
                destination += bt::DirSeparator();

            destination += tc->getStats().torrent_name + QLatin1String(".torrent");
            KIO::copy(QUrl::fromLocalFile(torFile), QUrl::fromLocalFile(destination));
        }

        // add torrent to group if necessary
        Group* g = gman->find(selected_group);
        if (g)
        {
            if (!g->isMember(tc))
            {
                g->addTorrent(tc, true);
                gman->saveGroups();
            }
        }

        torrentAdded(tc);
        if (silently)
            emit openedSilently(tc);
        return true;
    }

    bt::TorrentInterface* Core::loadFromData(const QByteArray& data, const QString& dir, const QString& group, bool silently, const QUrl& url)
    {
        QString tdir = findNewTorrentDir();
        TorrentControl* tc = 0;
        try
        {
            tc = new TorrentControl();
            tc->setLoadUrl(url);
            tc->init(qman, data, tdir, dir);

            if (init(tc, group, dir, silently))
            {
                startUpdateTimer();
                return tc;
            }
        }
        catch (bt::Warning& warning)
        {
            bt::Out(SYS_GEN | LOG_NOTICE) << warning.toString() << endl;
            canNotLoadSilently(warning.toString());
        }
        catch (bt::Error& err)
        {
            bt::Out(SYS_GEN | LOG_IMPORTANT) << err.toString() << endl;
            if (!silently)
                gui->errorMsg(err.toString());
            else
                canNotLoadSilently(err.toString());
        }

        delete tc;
        tc = 0;
        // delete tdir if necesarry
        if (bt::Exists(tdir))
            bt::Delete(tdir, true);

        return 0;
    }

    bt::TorrentInterface* Core::loadFromFile(const QString& target, const QString& dir, const QString& group, bool silently)
    {
        try
        {
            QByteArray data = bt::LoadFile(target);
            return loadFromData(data, dir, group, silently, QUrl::fromLocalFile(target));
        }
        catch (bt::Error& err)
        {
            bt::Out(SYS_GEN | LOG_IMPORTANT) << err.toString() << endl;
            if (!silently)
                gui->errorMsg(err.toString());
            else
                canNotLoadSilently(err.toString());
            return 0;
        }
    }

    void Core::downloadFinished(KJob* job)
    {
        KIO::StoredTransferJob* j = (KIO::StoredTransferJob*)job;
        int err = j->error();
        if (err == KIO::ERR_USER_CANCELED)
            return;

        if (err)
        {
            gui->errorMsg(j);
        }
        else
        {
            // load in the file (target is always local)
            QString group;
            QMap<QUrl, QString>::iterator i = add_to_groups.find(j->url());
            if (i != add_to_groups.end())
            {
                group = i.value();
                add_to_groups.erase(i);
            }

            QString dir = locationHint(group);
            if (dir != QString::null)
                loadFromData(j->data(), dir, group, false, j->url());
        }
    }

    void Core::load(const QUrl& url, const QString& group)
    {
        if (url.scheme() == QLatin1String("magnet"))
        {
            MagnetLinkLoadOptions options;
            options.silently = false;
            options.group = group;
            load(bt::MagnetLink(url), options);
        }
        else if (url.isLocalFile())
        {
            QString path = url.toLocalFile();
            QString dir = locationHint(group);
            if (dir != QString::null)
                loadFromFile(path, dir, group, false);
        }
        else
        {
            KIO::Job* j = KIO::storedGet(url);
            connect(j, &KIO::Job::result, this, &Core::downloadFinished);
            if (!group.isEmpty())
                add_to_groups.insert(url, group);
        }
    }

    void Core::downloadFinishedSilently(KJob* job)
    {
        KIO::StoredTransferJob* j = (KIO::StoredTransferJob*)job;
        int err = j->error();
        if (err == KIO::ERR_USER_CANCELED)
        {
            // do nothing
        }
        else if (err)
        {
            canNotLoadSilently(j->errorString());
        }
        else
        {
            QString dir;
            if (custom_save_locations.contains(j))
            {
                // we have a custom save location so save to that
                dir = custom_save_locations[j].toLocalFile();
                custom_save_locations.remove(j);
            }
            else if (!Settings::useSaveDir())
            {
                // incase save dir is not set, use home director
                Out(SYS_GEN | LOG_NOTICE) << "Cannot load " << j->url() << " silently, default save location not set !" << endl;
                Out(SYS_GEN | LOG_NOTICE) << "Using home directory instead !" << endl;
                dir = QDir::homePath();
            }
            else
            {
                dir = Settings::saveDir();
            }

            QString group;
            QMap<QUrl, QString>::iterator i = add_to_groups.find(j->url());
            if (i != add_to_groups.end())
            {
                group = i.value();
                add_to_groups.erase(i);
            }


            if (dir != QString::null)
                loadFromData(j->data(), dir, group, true, j->url());
        }
    }

    void Core::loadSilently(const QUrl &url, const QString& group)
    {
        if (url.scheme() == QLatin1String("magnet"))
        {
            MagnetLinkLoadOptions options;
            options.silently = true;
            options.group = group;
            load(bt::MagnetLink(url), options);
        }
        else if (url.isLocalFile())
        {
            QString path = url.toLocalFile();
            QString dir = locationHint(group);

            if (dir != QString::null)
                loadFromFile(path, dir, group, true);
        }
        else
        {
            // download to a random file in tmp
            KIO::Job* j = KIO::storedGet(url);
            connect(j, &KIO::Job::result, this, &Core::downloadFinishedSilently);
            if (!group.isEmpty())
                add_to_groups.insert(url, group);
        }
    }

    bt::TorrentInterface* Core::load(const QByteArray& data, const QUrl &url, const QString& group, const QString& savedir)
    {
        QString dir;
        if (savedir.isEmpty() || !bt::Exists(savedir))
            dir = locationHint(group);
        else
            dir = savedir;

        if (dir != QString::null)
            return loadFromData(data, dir, group, false, url);
        else
            return 0;
    }

    bt::TorrentInterface* Core::loadSilently(const QByteArray& data, const QUrl &url, const QString& group, const QString& savedir)
    {
        QString dir;
        if (savedir.isEmpty() || !bt::Exists(savedir))
            dir = locationHint(group);
        else
            dir = savedir;

        if (dir != QString::null)
            return loadFromData(data, dir, group, true, url);
        else
            return 0;
    }

    void Core::start(bt::TorrentInterface* tc)
    {
        if (tc->getStats().paused)
        {
            tc->unpause();
        }
        else
        {
            TorrentStartResponse reason = qman->start(tc);
            if (reason == NOT_ENOUGH_DISKSPACE || reason == QM_LIMITS_REACHED)
                canNotStart(tc, reason);
        }

        startUpdateTimer(); // restart update timer
    }

    void Core::start(QList<bt::TorrentInterface*> & todo)
    {
        if (todo.isEmpty())
            return;

        // unpause paused torrents
        for (QList<bt::TorrentInterface*>::iterator i = todo.begin(); i != todo.end();)
        {
            if ((*i)->getStats().paused)
            {
                (*i)->unpause();
                i = todo.erase(i);
            }
            else
                i++;
        }

        if (todo.count() == 1)
        {
            start(todo.front());
        }
        else
        {
            qman->start(todo);
        }

        startUpdateTimer(); // restart update timer
    }

    void Core::stop(bt::TorrentInterface* tc)
    {
        qman->stop(tc);
    }

    void Core::stop(QList<bt::TorrentInterface*> & todo)
    {
        qman->stop(todo);
    }

    void Core::pause(TorrentInterface* tc)
    {
        tc->pause();
    }

    void Core::pause(QList<bt::TorrentInterface*>& todo)
    {
        for (bt::TorrentInterface* tc : qAsConst(todo))
        {
            tc->pause();
        }
    }

    QString Core::findNewTorrentDir() const
    {
        int i = 0;
        while (true)
        {
            QDir d;
            QString dir = data_dir % QLatin1String("tor") % QString::number(i) % QLatin1Char('/');
            if (!d.exists(dir))
            {
                return dir;
            }
            i++;
        }
        return QString::null;
    }

    void Core::loadExistingTorrent(const QString& tor_dir)
    {
        TorrentControl* tc = 0;

        QString idir = tor_dir;
        if (!idir.endsWith(bt::DirSeparator()))
            idir += bt::DirSeparator();

        if (!bt::Exists(idir + QLatin1String("torrent")))
            return;

        try
        {
            tc = new TorrentControl();
            tc->init(qman, bt::LoadFile(idir + QLatin1String("torrent")), idir, QString::null);

            qman->append(tc);
            connectSignals(tc);
            torrentAdded(tc);
        }
        catch (bt::Error& err)
        {
            gui->errorMsg(err.toString());
            delete tc;
        }
        catch (bt::Warning& warning)
        {
            bt::Out(SYS_GEN | LOG_NOTICE) << warning.toString() << endl;
            canNotLoadSilently(warning.toString());
            bt::Delete(tor_dir, true);
        }
    }

    void Core::loadTorrents()
    {
        QDir dir(data_dir);
        QStringList filters;
        filters << QStringLiteral("tor*");
        QStringList sl = dir.entryList(filters, QDir::Dirs);
        for (int i = 0; i < sl.count(); i++)
        {
            QString idir = data_dir + sl.at(i);
            if (!idir.endsWith(DirSeparator()))
                idir.append(DirSeparator());

            Out(SYS_GEN | LOG_NOTICE) << "Loading " << idir << endl;
            loadExistingTorrent(idir);
        }

        gman->torrentsLoaded(qman);
        qman->loadState(KSharedConfig::openConfig());
        QTimer::singleShot(0, this, SLOT(delayedStart()));
    }

    void Core::delayedStart()
    {
        qman->orderQueue();
        if (!kt::QueueManager::enabled())
            qman->startAutoStartTorrents(); 
    }


    void Core::remove(bt::TorrentInterface* tc, bool data_to)
    {
        try
        {
            if (tc->getJobQueue()->runningJobs())
            {
                // if there are running jobs, schedule delete when they finish
                delayed_removal.insert(tc, data_to);
                connect(tc, &TorrentControl::runningJobsDone, this, &Core::delayedRemove);
                return;
            }

            const bt::TorrentStats& s = tc->getStats();
            removed_bytes_up += s.session_bytes_uploaded;
            removed_bytes_down += s.session_bytes_downloaded;
            stop(tc);

            QString dir = tc->getTorDir();

            try
            {
                if (data_to)
                    tc->deleteDataFiles();
            }
            catch (Error& e)
            {
                gui->errorMsg(e.toString());
            }

            torrentRemoved(tc);
            gman->torrentRemoved(tc);
            qman->torrentRemoved(tc);
            gui->updateActions();
            bt::Delete(dir, false);
            delayed_removal.remove(tc);
        }
        catch (Error& e)
        {
            gui->errorMsg(e.toString());
        }
    }

    void Core::remove(QList<bt::TorrentInterface*> & todo, bool data_to)
    {
        QList<bt::TorrentInterface*>::iterator i = todo.begin();
        while (i != todo.end())
        {
            bt::TorrentInterface* tc = *i;
            if (tc->getJobQueue()->runningJobs())
            {
                // if there are running jobs, schedule delete when they finish
                delayed_removal.insert(tc, data_to);
                connect(tc, &bt::TorrentInterface::runningJobsDone, this, &Core::delayedRemove);
                i = todo.erase(i);
            }
            else
                i++;
        }

        stop(todo);

        foreach (bt::TorrentInterface* tc, todo)
        {
            const bt::TorrentStats& s = tc->getStats();
            removed_bytes_up += s.session_bytes_uploaded;
            removed_bytes_down += s.session_bytes_downloaded;

            QString dir = tc->getTorDir();

            try
            {
                if (data_to)
                    tc->deleteDataFiles();
            }
            catch (Error& e)
            {
                gui->errorMsg(e.toString());
            }

            torrentRemoved(tc);
            gman->torrentRemoved(tc);
            try
            {
                bt::Delete(dir, false);
            }
            catch (Error& e)
            {
                gui->errorMsg(e.toString());
            }
        }

        qman->torrentsRemoved(todo);
        gui->updateActions();
    }

    void Core::delayedRemove(bt::TorrentInterface* tc)
    {
        if (!delayed_removal.contains(tc))
            return;

        remove(tc, delayed_removal[tc]);
    }

    void Core::setMaxDownloads(int max)
    {
        qman->setMaxDownloads(max);
    }

    void Core::setMaxSeeds(int max)
    {
        qman->setMaxSeeds(max);
    }

    void Core::torrentFinished(bt::TorrentInterface* tc)
    {
        if (!keep_seeding)
            tc->stop();

        finished(tc);
        qman->torrentFinished(tc);
    }

    void Core::setKeepSeeding(bool ks)
    {
        keep_seeding = ks;
        qman->setKeepSeeding(ks);
    }

    void Core::onExit()
    {
        emit aboutToQuit();
        // stop timer to prevent updates during wait
        exiting = true;
        update_timer.stop();

        net::SocketMonitor::instance().shutdown();
        mman->saveMagnets(kt::DataDir() + QLatin1String("magnets"));
        // make sure DHT is stopped
        Globals::instance().getDHT().stop();
        // stop all authentications going on
        AuthenticationMonitor::instance().shutdown();

        WaitJob* job = new WaitJob(5000);
        qman->saveState(KSharedConfig::openConfig());

        // Sync the config to be sure everything is saved
        Settings::self()->save();

        qman->onExit(job);
        // wait for completion of stopped events
        if (job->needToWait())
        {
            WaitJob::execute(job);
        }
        else
            delete job;

        // shutdown the servers
        Globals::instance().shutdownTCPServer();
        Globals::instance().shutdownUTPServer();

        pman->unloadAll();
        qman->clear();
    }

    bool Core::changeDataDir(const QString& new_dir)
    {
        try
        {
            // do nothing if new and old dir are the same
            if (QFileInfo(data_dir).absoluteFilePath().length() && QFileInfo(data_dir).absoluteFilePath() == QFileInfo(new_dir).absoluteFilePath()
                || data_dir == new_dir || data_dir == (new_dir + bt::DirSeparator()))
                return true;

            update_timer.stop();
            // safety check
            if (!bt::Exists(new_dir))
                bt::MakeDir(new_dir);


            // make sure new_dir ends with a /
            QString nd = new_dir;
            if (!nd.endsWith(DirSeparator()))
                nd += DirSeparator();

            Out(SYS_GEN | LOG_DEBUG) << "Switching to datadir " << nd << endl;

            qman->setSuspendedState(true);

            QList<bt::TorrentInterface*> succes;

            QList<bt::TorrentInterface*>::iterator i = qman->begin();
            while (i != qman->end())
            {
                bt::TorrentInterface* tc = *i;
                if (!tc->changeTorDir(nd))
                {
                    // failure time to roll back all the successful tc's
                    rollback(succes);
                    // set back the old data_dir in Settings
                    Settings::setTempDir(data_dir);
                    Settings::self()->save();
                    qman->setSuspendedState(false);
                    update_timer.start(CORE_UPDATE_INTERVAL);
                    return false;
                }
                else
                {
                    succes.append(tc);
                }
                i++;
            }
            data_dir = nd;
            qman->setSuspendedState(false);
            update_timer.start(CORE_UPDATE_INTERVAL);
            return true;
        }
        catch (bt::Error& e)
        {
            Out(SYS_GEN | LOG_IMPORTANT) << "Error : " << e.toString() << endl;
            update_timer.start(CORE_UPDATE_INTERVAL);
            return false;
        }
    }

    void Core::rollback(const QList<bt::TorrentInterface*> & succes)
    {
        Out(SYS_GEN | LOG_DEBUG) << "Error, rolling back" << endl;
        update_timer.stop();
        QList<bt::TorrentInterface*>::const_iterator i = succes.begin();
        while (i != succes.end())
        {
            (*i)->rollback();
            i++;
        }
        update_timer.start(CORE_UPDATE_INTERVAL);
    }

    void Core::startAll()
    {
        qman->startAll();
        startUpdateTimer();
    }

    void Core::stopAll()
    {
        qman->stopAll();
    }

    void Core::startUpdateTimer()
    {
        if (!update_timer.isActive())
        {
            Out(SYS_GEN | LOG_DEBUG) << "Started update timer" << endl;
            update_timer.start(CORE_UPDATE_INTERVAL);
            if (Settings::suppressSleep() && sleep_suppression_cookie == 0)
            {
                org::freedesktop::PowerManagement::Inhibit powerManagement(QStringLiteral("org.freedesktop.PowerManagement.Inhibit"), QStringLiteral("/org/freedesktop/PowerManagement/Inhibit"), QDBusConnection::sessionBus());
                QDBusPendingReply<quint32> pendingReply = powerManagement.Inhibit(QStringLiteral("ktorrent"), i18n("KTorrent is running one or more torrents"));
                auto pendingCallWatcher = new QDBusPendingCallWatcher(pendingReply, this);
                connect(pendingCallWatcher, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *callWatcher) {
                    QDBusPendingReply<quint32> reply = *callWatcher;
                    if (reply.isValid()) {
                        sleep_suppression_cookie = reply.value();
                        Out(SYS_GEN | LOG_DEBUG) << "Suppressing sleep" << endl;
                    }
                    else
                        Out(SYS_GEN | LOG_IMPORTANT) << "Failed to suppress sleeping" << endl;
                });
            }
        }
    }

    void Core::update()
    {
        if (exiting)
            return;

        try
        {
            bt::UpdateCurrentTime();
            AuthenticationMonitor::instance().update();

            QList<bt::TorrentInterface*>::iterator i = qman->begin();
            bool updated = false;
            while (i != qman->end())
            {
                bt::TorrentInterface* tc = *i;
                if (tc->getStats().running)
                {
                    tc->update();
                    updated = true;
                }
                i++;
            }

            if (!updated && mman->count() == 0)
            {
                Out(SYS_GEN | LOG_DEBUG) << "Stopped update timer" << endl;
                update_timer.stop(); // stop timer when not necessary

                if (sleep_suppression_cookie)
                {
                    org::freedesktop::PowerManagement::Inhibit powerManagement(QStringLiteral("org.freedesktop.PowerManagement.Inhibit"), QStringLiteral("/org/freedesktop/PowerManagement/Inhibit"), QDBusConnection::sessionBus());
                    auto pendingReply = powerManagement.UnInhibit(sleep_suppression_cookie);
                    auto pendingCallWatcher = new QDBusPendingCallWatcher(pendingReply, this);
                    connect(pendingCallWatcher, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *callWatcher) {
                        QDBusPendingReply<void> reply = *callWatcher;
                        if (reply.isValid()) {
                            sleep_suppression_cookie = 0;
                            Out(SYS_GEN | LOG_DEBUG) << "Stopped suppressing sleep" << endl;
                        }
                        else
                            Out(SYS_GEN | LOG_IMPORTANT) << "Failed to stop suppressing sleep" << endl;
                    });
                }
            }
            else
            {
                mman->update();
                // check if the priority of stalled torrents must be decreased
                if (Settings::decreasePriorityOfStalledTorrents())
                {
                    qman->checkStalledTorrents(bt::CurrentTime(), Settings::stallTimer());
                }
            }
        }
        catch (bt::Error& err)
        {
            Out(SYS_GEN | LOG_IMPORTANT) << "Caught bt::Error: " << err.toString() << endl;
        }
    }

    bt::TorrentInterface* Core::createTorrent(bt::TorrentCreator* mktor, bool seed)
    {
        QString tdir;
        try
        {
            tdir = findNewTorrentDir();
            bt::TorrentControl* tc = mktor->makeTC(tdir);
            if (tc)
            {
                connectSignals(tc);
                qman->append(tc);
                if (seed)
                    start(tc);
                torrentAdded(tc);
                return tc;
            }
        }
        catch (bt::Error& e)
        {
            // cleanup if necessary
            if (bt::Exists(tdir))
                bt::Delete(tdir, true);

            // Show error message
            gui->errorMsg(i18n("Cannot create torrent: %1", e.toString()));
        }
        return 0;
    }


    CurrentStats Core::getStats()
    {
        CurrentStats stats;
        Uint64 bytes_dl = 0, bytes_ul = 0;
        Uint32 speed_dl = 0, speed_ul = 0;


        for (QList<bt::TorrentInterface*>::iterator i = qman->begin(); i != qman->end(); ++i)
        {
            bt::TorrentInterface* tc = *i;
            const TorrentStats& s = tc->getStats();
            speed_dl += s.download_rate;
            speed_ul += s.upload_rate;
            bytes_dl += s.session_bytes_downloaded;
            bytes_ul += s.session_bytes_uploaded;
        }
        stats.download_speed = speed_dl;
        stats.upload_speed = speed_ul;
        stats.bytes_downloaded = bytes_dl + removed_bytes_down;
        stats.bytes_uploaded = bytes_ul + removed_bytes_up;

        return stats;
    }

    bool Core::changePort(Uint16 port)
    {
        bool ok = false;
        if (Settings::utpEnabled())
        {
            utp::UTPServer& utp_srv = Globals::instance().getUTPServer();
            ok = utp_srv.changePort(port);
            if (!Settings::onlyUseUtp())
            {
                bt::Server& srv = Globals::instance().getTCPServer();
                ok = ok && srv.changePort(port);
            }
        }
        else
        {
            bt::Server& srv = Globals::instance().getTCPServer();
            ok = srv.changePort(port);
        }

        return ok;
    }

    void Core::slotStoppedByError(bt::TorrentInterface* tc, QString msg)
    {
        emit torrentStoppedByError(tc, msg);
    }

    Uint32 Core::getNumTorrentsRunning() const
    {
        return qman->getNumRunning();
    }

    Uint32 Core::getNumTorrentsNotRunning() const
    {
        return qman->count() - qman->getNumRunning();
    }

    kt::QueueManager* Core::getQueueManager()
    {
        return this->qman;
    }

    void Core::torrentSeedAutoStopped(bt::TorrentInterface* tc, AutoStopReason reason)
    {
        qman->startNext();
        if (reason ==  MAX_RATIO_REACHED)
            maxShareRatioReached(tc);
        else
            maxSeedTimeReached(tc);
        startUpdateTimer();
    }

    void Core::setSuspendedState(bool suspend)
    {
        qman->setSuspendedState(suspend);
        if (!suspend)
            startUpdateTimer();
    }

    bool Core::getSuspendedState()
    {
        return qman->getSuspendedState();
    }

    bool Core::checkMissingFiles(TorrentInterface* tc)
    {
        QStringList missing;
        if (!tc->hasMissingFiles(missing))
            return true;

        QStringList not_mounted;
        while (!tc->isStorageMounted(not_mounted))
        {
            QString msg = i18n("One or more storage volumes are not mounted. In order to start this torrent, they need to be mounted.");
            KGuiItem retry(i18n("Retry"), QStringLiteral("emblem-mounted"));
            if (KMessageBox::warningContinueCancelList(gui, msg, not_mounted, QString(), retry) == KMessageBox::Continue)
            {
                not_mounted.clear();
                continue;
            }
            else
            {
                if (not_mounted.size() == 1)
                    tc->handleError(i18n("Storage volume %1 is not mounted", not_mounted.first()));
                else
                    tc->handleError(i18n("Storage volumes %1 are not mounted", not_mounted.join(QStringLiteral(", "))));
                return false;
            }
        }

        missing.clear();
        if (!tc->hasMissingFiles(missing))
            return true;

        if (tc->getStats().multi_file_torrent)
        {
            QString msg = i18n(
                              "Several data files of the torrent \"%1\" are missing. \n"
                              "Do you want to recreate them, or do you want to not download them?",
                              tc->getStats().torrent_name);

            MissingFilesDlg dlg(msg, missing, tc, 0);

            switch (dlg.execute())
            {
            case MissingFilesDlg::CANCEL:
                tc->handleError(i18n("Data files are missing"));
                return false;
            case MissingFilesDlg::DO_NOT_DOWNLOAD:
                try
                {
                    // mark them as do not download
                    tc->dndMissingFiles();
                }
                catch (bt::Error& e)
                {
                    gui->errorMsg(i18n("Cannot deselect missing files: %1", e.toString()));
                    tc->handleError(i18n("Data files are missing"));
                    return false;
                }
                break;
            case MissingFilesDlg::RECREATE:
                try
                {
                    // recreate them
                    tc->recreateMissingFiles();
                }
                catch (bt::Error& e)
                {
                    KMessageBox::error(0, i18n("Cannot recreate missing files: %1", e.toString()));
                    tc->handleError(i18n("Data files are missing"));
                    return false;
                }
                break;
            case MissingFilesDlg::NEW_LOCATION_SELECTED:
                break;
            }
        }
        else
        {
            QString msg = i18n("The file where the data is saved of the torrent \"%1\" is missing.\n"
                               "Do you want to recreate it?", tc->getStats().torrent_name);
            MissingFilesDlg dlg(msg, missing, tc, 0);

            switch (dlg.execute())
            {
            case MissingFilesDlg::CANCEL:
                tc->handleError(i18n("Data file is missing"));
                return false;
            case MissingFilesDlg::RECREATE:
                try
                {
                    tc->recreateMissingFiles();
                }
                catch (bt::Error& e)
                {
                    gui->errorMsg(i18n("Cannot recreate data file: %1", e.toString()));
                    tc->handleError(i18n("Data file is missing"));
                    return false;
                }
                break;
            case MissingFilesDlg::DO_NOT_DOWNLOAD:
                return false;
            case MissingFilesDlg::NEW_LOCATION_SELECTED:
                break;
            }
        }

        return true;
    }


    void Core::aboutToBeStarted(bt::TorrentInterface* tc, bool& ret)
    {
        ret = checkMissingFiles(tc);
    }

    void Core::emitCorruptedData(bt::TorrentInterface* tc)
    {
        corruptedData(tc);

    }

    void Core::connectSignals(bt::TorrentInterface* tc)
    {
        connect(tc, &bt::TorrentInterface::finished, this, &Core::torrentFinished);
        connect(tc, &bt::TorrentInterface::stoppedByError, this, &Core::slotStoppedByError);
        connect(tc, &bt::TorrentInterface::seedingAutoStopped, this, &Core::torrentSeedAutoStopped);
        connect(tc, &bt::TorrentInterface::aboutToBeStarted, this, &Core::aboutToBeStarted);
        connect(tc, &bt::TorrentInterface::corruptedDataFound, this, &Core::emitCorruptedData);
        connect(tc, &bt::TorrentInterface::needDataCheck, this, &Core::autoCheckData);
        connect(tc, &bt::TorrentInterface::statusChanged, this, &Core::onStatusChanged);
    }

    float Core::getGlobalMaxShareRatio() const
    {
        return Settings::maxRatio();
    }

    void Core::enqueueTorrentOverMaxRatio(bt::TorrentInterface* tc)
    {
        emit queuingNotPossible(tc);
    }

    void Core::autoCheckData(bt::TorrentInterface* tc)
    {
        Out(SYS_GEN | LOG_IMPORTANT) << "Doing an automatic data check on "
                                     << tc->getStats().torrent_name << endl;

        doDataCheck(tc);
    }

    void Core::doDataCheck(bt::TorrentInterface* tc, bool auto_import)
    {
        tc->startDataCheck(auto_import, 0, tc->getStats().total_chunks);
    }


    void Core::onLowDiskSpace(bt::TorrentInterface* tc, bool stopped)
    {
        emit lowDiskSpace(tc, stopped);
    }

    void Core::updateGuiPlugins()
    {
        pman->updateGuiPlugins();
    }

    DBus* Core::getExternalInterface()
    {
        return gui->getDBusInterface();
    }

    void Core::onStatusChanged(bt::TorrentInterface* tc)
    {
        Q_UNUSED(tc);
        if (!reordering_queue)
            gui->updateActions();
    }

    void Core::beforeQueueReorder()
    {
        reordering_queue = true;
    }

    void Core::afterQueueReorder()
    {
        reordering_queue = false;
        gui->updateActions();
        gman->updateCount(qman);
        startUpdateTimer();
    }

    void Core::customGroupChanged()
    {
        gman->updateCount(qman);
    }


    void Core::load(const bt::MagnetLink& mlink, const MagnetLinkLoadOptions& options)
    {
        if (!mlink.isValid())
        {
            gui->errorMsg(i18n("Invalid magnet bittorrent link: %1", mlink.toString()));
        }
        else
        {
            if (!Globals::instance().getDHT().isRunning())
                dhtNotEnabled(i18n("You are attempting to download a magnet link, and DHT is not enabled. "
                                   "For optimum results enable DHT."));
            mman->addMagnet(mlink, options, false);
            startUpdateTimer();
        }
    }

    void Core::onMetadataDownloaded(const bt::MagnetLink& mlink, const QByteArray& data, const kt::MagnetLinkLoadOptions& options)
    {
        QByteArray tmp;
        BEncoderBufferOutput* out = new BEncoderBufferOutput(tmp);
        BEncoder enc(out);
        enc.beginDict();
        QList<QUrl> trs = mlink.trackers();
        if (trs.count())
        {
            enc.write(QByteArrayLiteral("announce"));
            enc.write(trs.first().toDisplayString().toUtf8());
            if (trs.count() > 1)
            {
                enc.write(QByteArrayLiteral("announce-list"));
                enc.beginList();
                foreach (const QUrl &tracker, trs)
                {
                    enc.beginList();
                    enc.write(tracker.toDisplayString().toUtf8());
                    enc.end();
                }
                enc.end();
            }
        }
        enc.write(QByteArrayLiteral("info"));
        out->write(data.data(), data.size());
        enc.end();

        QUrl url(mlink.toString());

        bt::TorrentInterface* tc = 0;
        if (options.silently)
            tc = loadSilently(tmp, url, options.group, options.location);
        else
            tc = load(tmp, url, options.group, options.location);

        if (tc && !options.move_on_completion.isEmpty())
            tc->setMoveWhenCompletedDir(options.move_on_completion);
    }


    QString Core::locationHint(const QString& group) const
    {
        QString dir;

        // First see if we can use the group settings
        Group* g = gman->find(group);
        QString group_save_location = g != 0 ? g->groupPolicy().default_save_location : QString();
        if (!group_save_location.isEmpty() && bt::Exists(group_save_location))
            dir = g->groupPolicy().default_save_location;
        else if (Settings::useSaveDir())
            dir = Settings::saveDir();
        else
            dir = Settings::lastSaveDir();


        if (dir.isEmpty() || !bt::Exists(dir))
            dir = QDir::homePath();

        return dir;
    }

}
