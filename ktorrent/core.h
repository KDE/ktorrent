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
		virtual bool changeDataDir(const QString & new_dir);
		virtual void startAll(int type);
		virtual void stopAll(int type);
		virtual CurrentStats getStats();
		virtual bool changePort(bt::Uint16 port);
		virtual bt::Uint32 getNumTorrentsRunning() const;
		virtual bt::Uint32 getNumTorrentsNotRunning() const;
		virtual void load(const KUrl& url,const QString & group);
		virtual void load(const QByteArray & data,const KUrl& url,const QString & group,const QString & savedir);
		virtual void loadSilently(const KUrl& url,const QString & group);
		virtual void loadSilently(const QByteArray & data,const KUrl& url,const QString & group,const QString & savedir);
		virtual QString findNewTorrentDir() const;
		virtual void loadExistingTorrent(const QString & tor_dir);
		virtual void setPausedState(bool pause);
		virtual bool getPausedState();
		virtual float getGlobalMaxShareRatio() const;
		virtual QObject* getExternalInterface();

		/// Get the queue manager
		kt::QueueManager* getQueueManager();

		/// Get the group manager
		kt::GroupManager* getGroupManager() {return gman;}

		/**
		 * Make a torrent file
		 * @param file The file or dir to make a torrent of
		 * @param trackers A list of trackers
		 * @param webseeds List of webseed URL's
		 * @param chunk_size The size of each chunk (in KB)
		 * @param name The torrents name (usually filename)
		 * @param comments The comments
		 * @param seed Whether or not to start seeding or not
		 * @param output_file File to store the torrent file
		 * @param priv_tor Is this a private torrent
		 * @param prog Progress bar to update
		 * @return The created torrent
		 */
		bt::TorrentInterface* makeTorrent(const QString & file,const QStringList & trackers,const KUrl::List & webseeds,
				int chunk_size,const QString & name,const QString & comments,
				bool seed,const QString & output_file,bool priv_tor,QProgressBar* prog, bool decentralized);

		/**
		 * KT is exiting, shutdown the core
		 */
		void onExit();

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
		 * Load a torrent file. Pops up an error dialog
		 * if something goes wrong.
		 * @param file The torrent file (always a local file)
		 * @param dir Directory to save the data
		 * @param group Group to add torrent to
		 * @param silently Whether or not to do this silently
		 */
		bool load(const QString & file,const QString & dir,const QString & group,bool silently);
		
		/**
		 * Load a torrent file. Pops up an error dialog
		 * if something goes wrong.
		 * @param data Byte array of the torrent file
		 * @param dir Directory to save the data
		 * @param silently Whether or not to do this silently
		 */
		bool load(const QByteArray & data,const QString & dir,const QString & group,bool silently, const KUrl& url);
		
		/**
		 * Remove a download.This will delete all temp
		 * data from this TorrentInterface And delete the
		 * TorrentInterface itself. It can also potentially
		 * start a new download (when one is waiting to be downloaded).
		 * @param tc
		 * @param data_to 
		 */
		virtual void remove(bt::TorrentInterface* tc,bool data_to);

		/**
		 * Update all torrents.
		 */
		void update();
		
		/**
		 * Start a torrent, takes into account the maximum number of downloads.
		 * @param tc The TorrentInterface
		 */
		virtual void start(bt::TorrentInterface* tc);

		/**
		 * Start a list of torrents
		 * @param todo The list of torrents
		 */
		virtual void start(QList<bt::TorrentInterface*> & todo);
		
		/**
		 * Stop a torrent, may start another download if it hasn't been started.
		 * @param tc The TorrentInterface
		 */
		virtual void stop(bt::TorrentInterface* tc, bool user = false);
		
		/**
		 * Enqueue/Dequeue function. Places a torrent in queue. 
		 * If the torrent is already in queue this will remove it from queue.
		 * @param tc TorrentControl pointer.
		 */
		virtual void queue(bt::TorrentInterface* tc);
		
		/**
		 * A torrent is about to be started. We will do some file checks upon this signal.
		 * @param tc The TorrentInterface
		*/
		void aboutToBeStarted(bt::TorrentInterface* tc,bool & ret);
		
		/**
		 * User tried to enqueue a torrent that has reached max share ratio.
		 * Emits appropriate signal.
		 */
		void enqueueTorrentOverMaxRatio(bt::TorrentInterface* tc);
		
		/**
		 * Do a data check on a torrent
		 * @param tc 
		 */
		void doDataCheck(bt::TorrentInterface* tc);
		
		///Fires when disk space is running low
		void onLowDiskSpace(bt::TorrentInterface* tc, bool stopped);

		/// Apply all the settings
		void applySettings();

		/// Update the GUI plugins
		void updateGuiPlugins();
		
		/// Import KDE3 torrents
		void importKDE3Torrents();

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
		void canNotStart(bt::TorrentInterface* tc,bt::TorrentStartResponse reason);

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
		void canNotLoadSilently(const QString & msg);

	private:
		void rollback(const QList<bt::TorrentInterface*> & success);
		void connectSignals(bt::TorrentInterface* tc);
		bool init(bt::TorrentControl* tc,const QString & group,bool silently);
		void applySettings(bool change_port);

	public:
		void loadTorrents();
		
	private slots:
		void torrentFinished(bt::TorrentInterface* tc);
		void slotStoppedByError(bt::TorrentInterface* tc, QString msg);
		void torrentSeedAutoStopped(bt::TorrentInterface* tc,bt::AutoStopReason reason);
		void downloadFinished(KJob *job);
		void downloadFinishedSilently(KJob *job);
		void emitCorruptedData(bt::TorrentInterface* tc);
		void autoCheckData(bt::TorrentInterface* tc);
		void checkForKDE3Torrents();

	private:
		GUI* gui;
		bool keep_seeding;
		QString data_dir;
		QTimer update_timer;
		bt::Uint64 removed_bytes_up,removed_bytes_down;
		kt::PluginManager* pman;
		kt::QueueManager* qman;
		kt::GroupManager* gman;
		QMap<KJob*,KUrl> custom_save_locations; // map to store save locations
		QMap<KJob*,QString> add_to_groups; // Map to keep track of which group to add a torrent to
		int sleep_suppression_cookie;
	};
}

#endif

