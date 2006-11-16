/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
#ifndef KTCOREINTERFACE_H
#define KTCOREINTERFACE_H

#include <kurl.h>
#include <qobject.h>
#include <util/constants.h>
#include <torrent/queuemanager.h>

///Stats struct
struct CurrentStats
{
	bt::Uint32 download_speed;
	bt::Uint32 upload_speed;
	bt::Uint64 bytes_downloaded;
	bt::Uint64 bytes_uploaded;
};

namespace bt
{
	class QueueManager;
}
namespace kt
{
	class TorrentInterface;

	/**
	 * @author Joris Guisson
	 * @brief Interface for plugins to communicate with the application's core
	 *
	 * This interface provides the plugin with the functionality to modify
	 * the applications core, the core is responsible for managing all
	 * TorrentControl objects.
	*/
	class CoreInterface : public QObject
	{
		Q_OBJECT
	public:
		CoreInterface();
		virtual ~CoreInterface();

		/**
		 * Set the maximum number of simultanious downloads.
		 * @param max The max num (0 == no limit)
		 */
		virtual void setMaxDownloads(int max) = 0;
	
		virtual void setMaxSeeds(int max) = 0;

		virtual void setMaxDownloadSpeed(int v) = 0;
		virtual void setMaxUploadSpeed(int v) = 0;

		/**
		 * Set wether or not we should keep seeding after
		 * a download has finished.
		 * @param ks Keep seeding yes or no
		 */
		virtual void setKeepSeeding(bool ks) = 0;
	
		/**
		 * Change the data dir. This involves copying
		 * all data from the old dir to the new.
		 * This can offcourse go horribly wrong, therefore
		 * if it doesn't succeed it returns false
		 * and leaves everything where it supposed to be.
		 * @param new_dir The new directory
		 */
		virtual bool changeDataDir(const QString & new_dir) = 0;

		/**
		 * Start all, takes into account the maximum number of downloads.
		 * @param type - Weather to start downloads, seeds or both. 1=Downloads, 2=Seeds, 3=All
		 */
		virtual void startAll(int type) = 0;

		/**
		 * Stop all torrents.
		 * @param type - Weather to start downloads, seeds or both. 1=Downloads, 2=Seeds, 3=All
		 */
		virtual void stopAll(int type) = 0;

		/**
		 * Start a torrent, takes into account the maximum number of downloads.
		 * @param tc The TorrentControl
	 	 */
		virtual void start(TorrentInterface* tc) = 0;

		/**
		 * Stop a torrent, may start another download if it hasn't been started.
		 * @param tc The TorrentControl
		 * @param user true if user stopped the torrent, false otherwise
		 */
		virtual void stop(TorrentInterface* tc, bool user = false) = 0;
		
		/**
		 * Enqueue/Dequeue function. Places a torrent in queue. 
		 * If the torrent is already in queue this will remove it from queue.
		 * @param tc TorrentControl pointer.
		 */
		virtual void queue(kt::TorrentInterface* tc) = 0;

		virtual bt::QueueManager* getQueueManager() = 0;

		virtual CurrentStats getStats() = 0;

		/**
		 * Switch the port when no torrents are running.
		 * @param port The new port
		 * @return true if we can, false if there are torrents running
		 */
		virtual bool changePort(bt::Uint16 port) = 0;

		///  Get the number of torrents running (including seeding torrents).
		virtual bt::Uint32 getNumTorrentsRunning() const = 0;

		///  Get the number of torrents not running.
		virtual bt::Uint32 getNumTorrentsNotRunning() const = 0;

		/**
		 * Load a torrent file. Pops up an error dialog
		 * if something goes wrong.
		 * @param file The torrent file
		 * @param savedir Dir to save the data
		 * @param silently Wether or not to do this silently
		 */
		virtual bool load(const QString & file,const QString & savedir,bool silently) = 0;

		/**
		 * Load a torrent file. Pops up an error dialog
		 * if something goes wrong. Will ask the user for a save location, or use
		 * the default.
		 * @param url The torrent file
		 */
		virtual void load(const KURL& url) = 0;
		
		/**
		 * Load a torrent file. Pops up an error dialog
		 * if something goes wrong. Will ask the user for a save location, or use
		 * the default. This will not popup a file selection dialog for multi file torrents.
		 * @param url The torrent file
		 */
		virtual void loadSilently(const KURL& url) = 0;
		
		/**
		 * Remove a download.This will delete all temp
		 * data from this TorrentControl And delete the
		 * TorrentControl itself. It can also potentially
		 * start a new download (when one is waiting to be downloaded).
		 * @param tc The torrent
		 * @param data_to Wether or not to delete the file data to
		 */
		virtual void remove(TorrentInterface* tc,bool data_to) = 0;
		
		/**
		 * Inserts IP range to be blocked into IPBlocklist
		 * @param ip QString reference to single IP or IP range. For example:
		 * single - 127.0.0.5
		 * range - 127.0.*.*
		 **/
		virtual void addBlockedIP(QString& ip) = 0;
		
		/**
		 * Removes IP range from IPBlocklist
		 * @param ip QString reference to single IP or IP range. For example:
		 * single - 127.0.0.5
		 * range - 127.0.*.*
		 **/
		virtual void removeBlockedIP(QString& ip) = 0;
		
		/**
		 * Find the next free torX dir.
		 * @return Path to the dir (including the torX part)
		 */
		virtual QString findNewTorrentDir() const = 0;
		
		/**
		 * Load an existing torrent, which has already a properly set up torX dir.
		 * @param tor_dir The torX dir
		 */
		virtual void loadExistingTorrent(const QString & tor_dir) = 0;
		
		/**
		 * Returns maximum allowed download speed.
		 */
		virtual int getMaxDownloadSpeed() = 0;
		
		/**
		 * Returns maximum allowed upload speed.
		 */
		virtual int getMaxUploadSpeed() = 0;
		
		/**
		 * Sets global paused state for all torrents (QueueManager) and stopps all torrents.
		 * No torrents will be automatically started/stopped.
		 */
		virtual void setPausedState(bool pause) = 0;
		
		/// Get the global share ratio limit
		virtual float getGlobalMaxShareRatio() const = 0; 
		
	signals:
		/**
		 * Seeing that when load returns the loading process may not have finished yet,
		 * and some code expects this. We emit this signal to notify that code of it.
		 * @param url The url which has been loaded
		 * @param success Wether or not it succeeded
		 * @param canceled Wether or not it was canceled by the user
		 */
		void loadingFinished(const KURL & url,bool success,bool canceled);
		
		/**
		 * A TorrentInterface was added
		 * @param tc 
		 */
		void torrentAdded(kt::TorrentInterface* tc);

	
		/**
		 * A TorrentInterface was removed
		 * @param tc
		 */
		void torrentRemoved(kt::TorrentInterface* tc);
		
		/**
		 * A TorrentInterface has finished downloading.
		 * @param tc
		 */
		void finished(kt::TorrentInterface* tc);

    	/**
		 * Torrent download is stopped by error
		 * @param tc TorrentInterface
		 * @param msg Error message
		 */
		void torrentStoppedByError(kt::TorrentInterface* tc, QString msg);
	};

}

#endif
