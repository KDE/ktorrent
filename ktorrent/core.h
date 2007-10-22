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

namespace KIO
{
	class Job;
}

namespace bt
{
	class QueueManager;
	class TorrentControl;
}

namespace kt
{
	class GUIInterface;
	class PluginManager;
	class GroupManager;

	/**
	 * Core of ktorrent, manages every non GUI aspect of the application
	 * */
	class Core : public CoreInterface
	{
		Q_OBJECT
	public:
		Core(GUIInterface* gui);
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
		virtual void load(const KUrl& url);
		virtual void loadSilently(const KUrl& url);
		virtual void loadSilentlyDir(const KUrl& url,const KUrl & savedir);
		virtual QString findNewTorrentDir() const;
		virtual void loadExistingTorrent(const QString & tor_dir);
		virtual void setPausedState(bool pause);
		virtual float getGlobalMaxShareRatio() const;

		/// Get the queue manager
		bt::QueueManager* getQueueManager();

		/// Get the group manager
		kt::GroupManager* getGroupManager() {return gman;}

		/**
		 * Make a torrent file
		 * @param file The file or dir to make a torrent of
		 * @param trackers A list of trackers
		 * @param chunk_size The size of each chunk (in KB)
		 * @param name The torrents name (usually filename)
		 * @param comments The comments
		 * @param seed Wether or not to start seeding or not
		 * @param output_file File to store the torrent file
		 * @param priv_tor Is this a private torrent
		 * @param prog Progress bar to update
		 */
		void makeTorrent(const QString & file,const QStringList & trackers,
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
		 * Load a torrent file. Pops up an error dialog
		 * if something goes wrong.
		 * @param file The torrent file (always a local file)
		 * @param dir Directory to save the data
		 * @param silently Wether or not to do this silently
		 */
		bool load(const QString & file,const QString & dir,bool silently);
		
		/**
		 * Load a torrent file. Pops up an error dialog
		 * if something goes wrong.
		 * @param data Byte array of the torrent file
		 * @param dir Directory to save the data
		 * @param silently Wether or not to do this silently
		 */
		bool load(const QByteArray & data,const QString & dir,bool silently, const KUrl& url);
		
		/**
		 * Remove a download.This will delete all temp
		 * data from this TorrentInterface And delete the
		 * TorrentInterface itself. It can also potentially
		 * start a new download (when one is waiting to be downloaded).
		 * @param tc
		 * @param data_to 
		 */
		virtual void remove(kt::TorrentInterface* tc,bool data_to);

		/**
		 * Update all torrents.
		 */
		void update();
		
		/**
		 * Start a torrent, takes into account the maximum number of downloads.
		 * @param tc The TorrentInterface
		 */
		virtual void start(kt::TorrentInterface* tc);

		/**
		 * Stop a torrent, may start another download if it hasn't been started.
		 * @param tc The TorrentInterface
		 */
		virtual void stop(kt::TorrentInterface* tc, bool user = false);
		
		/**
		 * Enqueue/Dequeue function. Places a torrent in queue. 
		 * If the torrent is already in queue this will remove it from queue.
		 * @param tc TorrentControl pointer.
		 */
		virtual void queue(kt::TorrentInterface* tc);
		
		/**
		 * A torrent is about to be started. We will do some file checks upon this signal.
		 * @param tc The TorrentInterface
		*/
		void aboutToBeStarted(kt::TorrentInterface* tc,bool & ret);
		
		/**
		 * User tried to enqueue a torrent that has reached max share ratio.
		 * Emits appropriate signal.
		 */
		void enqueueTorrentOverMaxRatio(kt::TorrentInterface* tc);
		
		/**
		 * Do a data check on a torrent
		 * @param tc 
		 */
		void doDataCheck(kt::TorrentInterface* tc);
		
		///Fires when disk space is running low
		void onLowDiskSpace(kt::TorrentInterface* tc, bool stopped);

		/// Apply all the settings
		void applySettings();

		/// Update the GUI plugins
		void updateGuiPlugins();

	signals:
		/**
		* TorrentCore torrents have beed updated. Stats are changed.
		**/
		void statsUpdated();
		
		/**
		 * Emitted when a torrent has reached it's max share ratio.
		 * @param tc The torrent
		 */
		void maxShareRatioReached(kt::TorrentInterface* tc);

		/**
		 * Emitted when a torrent has reached it's max seed time
		 * @param tc The torrent
		 */
		void maxSeedTimeReached(kt::TorrentInterface* tc);
		
		/**
		 * Corrupted data has been detected.
		 * @param tc The torrent with the corrupted data
		 */
		void corruptedData(kt::TorrentInterface* tc);
		
		/**
		 * User tried to enqueue a torrent that has reached max share ratio. It's not possible.
		 * Signal should be connected to SysTray slot which shows appropriate KPassivePopup info.
		 * @param tc The torrent in question.
		 */
		void queuingNotPossible(kt::TorrentInterface* tc);

		/**
		 * Emitted when a torrent cannot be started
		 * @param tc The torrent
		 * @param reason The reason
		 */
		void canNotStart(kt::TorrentInterface* tc,kt::TorrentStartResponse reason);

		/**
		 * Diskspace is running low.
		 * Signal should be connected to SysTray slot which shows appropriate KPassivePopup info. 
		 * @param tc The torrent in question.
		 */
		void lowDiskSpace(kt::TorrentInterface* tc, bool stopped);

	private:
		void rollback(const QList<kt::TorrentInterface*> & success);
		void connectSignals(kt::TorrentInterface* tc);
		bool init(bt::TorrentControl* tc,bool silently);
		void applySettings(bool change_port);

	public:
		void loadTorrents();
		
	private slots:
		void torrentFinished(kt::TorrentInterface* tc);
		void slotStoppedByError(kt::TorrentInterface* tc, QString msg);
		void torrentSeedAutoStopped(kt::TorrentInterface* tc,kt::AutoStopReason reason);
		void downloadFinished(KIO::Job *job);
		void downloadFinishedSilently(KIO::Job *job);
		void emitCorruptedData(kt::TorrentInterface* tc);
		void autoCheckData(kt::TorrentInterface* tc);

	private:
		GUIInterface* gui;
		bool keep_seeding;
		QString data_dir;
		QTimer update_timer;
		bt::Uint64 removed_bytes_up,removed_bytes_down;
		kt::PluginManager* pman;
		bt::QueueManager* qman;
		kt::GroupManager* gman;
		QMap<KIO::Job*,KUrl> custom_save_locations; // map to store save locations
	};
}

#endif

