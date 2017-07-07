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

#include <QMap>
#include <QTimer>

#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>

class QDBusInterface;
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
        ~Core();

        // implemented from CoreInterface
        void setKeepSeeding(bool ks) override;
        bool changeDataDir(const QString& new_dir) override;
        void startAll() override;
        void stopAll() override;
        CurrentStats getStats() override;
        bool changePort(bt::Uint16 port) override;
        bt::Uint32 getNumTorrentsRunning() const override;
        bt::Uint32 getNumTorrentsNotRunning() const override;
        void load(const QUrl &url, const QString& group) override;
        bt::TorrentInterface* load(const QByteArray& data, const QUrl &url, const QString& group, const QString& savedir) override;
        void loadSilently(const QUrl &url, const QString& group) override;
        bt::TorrentInterface* loadSilently(const QByteArray& data, const QUrl &url, const QString& group, const QString& savedir) override;
        void load(const bt::MagnetLink& mlink, const MagnetLinkLoadOptions& options) override;
        QString findNewTorrentDir() const override;
        void loadExistingTorrent(const QString& tor_dir) override;
        void setSuspendedState(bool suspend) override;
        bool getSuspendedState() override;
        float getGlobalMaxShareRatio() const;
        DBus* getExternalInterface() override;

        /// Get the queue manager
        kt::QueueManager* getQueueManager() override;

        /// Get the group manager
        kt::GroupManager* getGroupManager() override {return gman;}

        /// Get the magnet manager
        kt::MagnetManager* getMagnetManager() override {return mman;}

         bt::TorrentInterface* createTorrent(bt::TorrentCreator* mktor, bool seed) override;

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

        void start(bt::TorrentInterface* tc) override;
        void start(QList<bt::TorrentInterface*> & todo) override;
        void stop(bt::TorrentInterface* tc) override;
        void stop(QList<bt::TorrentInterface*> & todo) override;
        void remove(bt::TorrentInterface* tc, bool data_to) override;
        void remove(QList<bt::TorrentInterface*> & todo, bool data_to) override;
        void pause(bt::TorrentInterface* tc) override;
        void pause(QList<bt::TorrentInterface*> & todo) override;

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
        void applySettings() override;

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

        /**
         * Emitted just before Core object is destroyed
        */
        void aboutToQuit();

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
        quint32 sleep_suppression_cookie;
        QMap<bt::TorrentInterface*, bool> delayed_removal;
        bool exiting;
        bool reordering_queue;
    };
}

#endif

