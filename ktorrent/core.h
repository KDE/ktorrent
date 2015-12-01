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
#ifndef KTCORE_HH
#define KTCORE_HH

#include <qmap.h>
#include <qtimer.h>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>

class QProgressBar;

class KJob;

namespace bt
{
    class TorrentControl;
    class TorrentInterface;
}

namespace kt
{
    class MagnetManager;
    class GUI;
    class PluginManager;
    class GroupManager;

    /**
     * Core of ktorrent, manages every non GUI aspect of the application
     * */
    class Core : public CoreInterface
    {
        Q_OBJECT
    public:
        Core(GUI* gui);
        virtual ~Core();

        // implemented from CoreInterface
        virtual void setKeepSeeding(bool ks);
        virtual bool changeDataDir(const QString& new_dir);
        virtual void startAll();
        virtual void stopAll();
        virtual CurrentStats getStats();
        virtual bool changePort(bt::Uint16 port);
        virtual bt::Uint32 getNumTorrentsRunning() const;
        virtual bt::Uint32 getNumTorrentsNotRunning() const;
        virtual void load(const QUrl &url, const QString& group);
        virtual bt::TorrentInterface* load(const QByteArray& data, const QUrl &url, const QString& group, const QString& savedir);
        virtual void loadSilently(const QUrl &url, const QString& group);
        virtual bt::TorrentInterface* loadSilently(const QByteArray& data, const QUrl &url, const QString& group, const QString& savedir);
        virtual void load(const bt::MagnetLink& mlink, const MagnetLinkLoadOptions& options);
        virtual QString findNewTorrentDir() const;
        virtual void loadExistingTorrent(const QString& tor_dir);
        virtual void setSuspendedState(bool suspend);
        virtual bool getSuspendedState();
        virtual float getGlobalMaxShareRatio() const;
        virtual DBus* getExternalInterface();

        /// Get the queue manager
        kt::QueueManager* getQueueManager();

        /// Get the group manager
        kt::GroupManager* getGroupManager() {return gman;}

        /// Get the magnet manager
        kt::MagnetManager* getMagnetManager() {return mman;}

        virtual  bt::TorrentInterface* createTorrent(bt::TorrentCreator* mktor, bool seed);

        /**
         * Set the maximum number of simultaneous downloads.
         * @param max The max num (0 == no limit)
         */
        void setMaxDownloads(int max);

        /**
         * Set the maximum number of simultaneous seeds.
         * @param max The max num (0 == no limit)
         */
        void setMaxSeeds(int max);

        /**
         * Load plugins.
         */
        void loadPlugins();

    public slots:
        /**
         * Start the update timer
         */
        void startUpdateTimer();

        /**
         * Update all torrents.
         */
        void update();

        virtual void start(bt::TorrentInterface* tc);
        virtual void start(QList<bt::TorrentInterface*> & todo);
        virtual void stop(bt::TorrentInterface* tc);
        virtual void stop(QList<bt::TorrentInterface*> & todo);
        virtual void remove(bt::TorrentInterface* tc, bool data_to);
        virtual void remove(QList<bt::TorrentInterface*> & todo, bool data_to);
        virtual void pause(bt::TorrentInterface* tc);
        virtual void pause(QList<bt::TorrentInterface*> & todo);

        /**
         * A torrent is about to be started. We will do some file checks upon this signal.
         * @param tc The TorrentInterface
        */
        void aboutToBeStarted(bt::TorrentInterface* tc, bool& ret);

        /**
         * Checks for missing files and shows the MissingFilesDlg if necessary.
         * @param tc The torrent
         * @return True if everything is OK, false otherwise
         */
        bool checkMissingFiles(bt::TorrentInterface* tc);

        /**
         * User tried to enqueue a torrent that has reached max share ratio.
         * Emits appropriate signal.
         */
        void enqueueTorrentOverMaxRatio(bt::TorrentInterface* tc);

        /**
         * Do a data check on a torrent
         * @param tc The torrent
         * @param auto_import Is this an automatic import
         */
        void doDataCheck(bt::TorrentInterface* tc, bool auto_import = false);

        ///Fires when disk space is running low
        void onLowDiskSpace(bt::TorrentInterface* tc, bool stopped);

        /// Apply all the settings
        void applySettings();

        /// Update the GUI plugins
        void updateGuiPlugins();

        /// Handle status changes
        void onStatusChanged(bt::TorrentInterface* tc);

        /// Handle the download of meta data
        void onMetadataDownloaded(const bt::MagnetLink& mlink, const QByteArray& data, const kt::MagnetLinkLoadOptions& options);

    signals:
        /**
        * TorrentCore torrents have beed updated. Stats are changed.
        **/
        void statsUpdated();

        /**
         * Emitted when a torrent has reached it's max share ratio.
         * @param tc The torrent
         */
        void maxShareRatioReached(bt::TorrentInterface* tc);

        /**
         * Emitted when a torrent has reached it's max seed time
         * @param tc The torrent
         */
        void maxSeedTimeReached(bt::TorrentInterface* tc);

        /**
         * Corrupted data has been detected.
         * @param tc The torrent with the corrupted data
         */
        void corruptedData(bt::TorrentInterface* tc);

        /**
         * User tried to enqueue a torrent that has reached max share ratio. It's not possible.
         * Signal should be connected to SysTray slot which shows appropriate KPassivePopup info.
         * @param tc The torrent in question.
         */
        void queuingNotPossible(bt::TorrentInterface* tc);

        /**
         * Emitted when a torrent cannot be started
         * @param tc The torrent
         * @param reason The reason
         */
        void canNotStart(bt::TorrentInterface* tc, bt::TorrentStartResponse reason);

        /**
         * Diskspace is running low.
         * Signal should be connected to SysTray slot which shows appropriate KPassivePopup info.
         * @param tc The torrent in question.
         */
        void lowDiskSpace(bt::TorrentInterface* tc, bool stopped);

        /**
         * Loading silently failed.
         * @param msg Error message
         */
        void canNotLoadSilently(const QString& msg);

        /**
         * Emitted when DHT is not enabled and a MagnetLink is being downloaded
        */
        void dhtNotEnabled(const QString& msg);
        void openedSilently(bt::TorrentInterface* tc);

    private:
        void rollback(const QList<bt::TorrentInterface*> & success);
        void connectSignals(bt::TorrentInterface* tc);
        bool init(bt::TorrentControl* tc, const QString& group, const QString& location, bool silently);
        QString locationHint(const QString& group) const;
        void startServers();
        void startTCPServer(bt::Uint16 port);
        void startUTPServer(bt::Uint16 port);
        bt::TorrentInterface* loadFromFile(const QString& file, const QString& dir, const QString& group, bool silently);
        bt::TorrentInterface* loadFromData(const QByteArray& data, const QString& dir, const QString& group, bool silently, const QUrl& url);

    public:
        void loadTorrents();

    private slots:
        void torrentFinished(bt::TorrentInterface* tc);
        void slotStoppedByError(bt::TorrentInterface* tc, QString msg);
        void torrentSeedAutoStopped(bt::TorrentInterface* tc, bt::AutoStopReason reason);
        void downloadFinished(KJob* job);
        void downloadFinishedSilently(KJob* job);
        void emitCorruptedData(bt::TorrentInterface* tc);
        void autoCheckData(bt::TorrentInterface* tc);
        void delayedRemove(bt::TorrentInterface* tc);
        void delayedStart();
        void beforeQueueReorder();
        void afterQueueReorder();
        void customGroupChanged();
        /**
         * KT is exiting, shutdown the core
         */
        void onExit();

    private:
        GUI* gui;
        bool keep_seeding;
        QString data_dir;
        QTimer update_timer;
        bt::Uint64 removed_bytes_up, removed_bytes_down;
        kt::PluginManager* pman;
        kt::QueueManager* qman;
        kt::GroupManager* gman;
        kt::MagnetManager* mman;
        QMap<KJob*, QUrl> custom_save_locations; // map to store save locations
        QMap<QUrl, QString> add_to_groups; // Map to keep track of which group to add a torrent to
        int sleep_suppression_cookie;
        QMap<bt::TorrentInterface*, bool> delayed_removal;
        bool exiting;
        bool reordering_queue;
    };
}

#endif

