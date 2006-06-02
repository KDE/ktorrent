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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef KTORRENTCORE_H
#define KTORRENTCORE_H

#include <qobject.h>
#include <qtimer.h>
#include <qcstring.h>
#include <util/constants.h>
#include <interfaces/coreinterface.h>

typedef QValueList<QCString> QCStringList;

namespace bt
{
	class Server;
	class QueueManager;
}

namespace kt
{
	class Plugin;
	class PluginManager;
	class GUIInterface;
	class TorrentInterface;
}


///Stats struct
struct CurrentStats
{
	bt::Uint32 download_speed;
	bt::Uint32 upload_speed;
	bt::Uint64 bytes_downloaded;
	bt::Uint64 bytes_uploaded;
};

class KProgress;

/**
 * @author Joris Guisson
 * @brief Keeps track of all TorrentInterface objects
 *
 * This class keeps track of all TorrentInterface objects.
 */
class KTorrentCore : public QObject,public kt::CoreInterface
{
	Q_OBJECT
public:
	KTorrentCore(kt::GUIInterface* gui);
	virtual ~KTorrentCore();
	

	kt::PluginManager & getPluginManager() {return *pman;}
	const kt::PluginManager & getPluginManager() const {return *pman;}
	
	/**
	 * Load all torrents from the data dir.
	 */
	void loadTorrents();
	
	/**
	 * Load an existing torrent, which has already a properly set up torX dir.
	 * @param tor_dir The torX dir
	 */
	void loadExistingTorrent(const QString & tor_dir);
	
	/**
	 * Set the maximum number of simultanious downloads.
	 * @param max The max num (0 == no limit)
	 */
	void setMaxDownloads(int max);
	
	/**
	 * Set the maximum number of simultaneous seeds.
	 * @param max The max num (0 == no limit)
	 */
	void setMaxSeeds(int max);
	
	/**
	 * Set wether or not we should keep seeding after
	 * a download has finished.
	 * @param ks Keep seeding yes or no
	 */
	void setKeepSeeding(bool ks);
	
	/**
	 * Change the data dir. This involves copying
	 * all data from the old dir to the new.
	 * This can offcourse go horribly wrong, therefore
	 * if it doesn't succeed it returns false
	 * and leaves everything where it supposed to be.
	 * @param new_dir The new directory
	 */
	bool changeDataDir(const QString & new_dir);
	
	/**
	 * Save active torrents on exit.
	 */
	void onExit();

	/**
	 * Start all, takes into account the maximum number of downloads.
	 * @param type Wether to start all downloads or seeds. 1=Downloads, 2=Seeds, 3=All
	 */
	void startAll(int type);

	/**
	 * Stop all torrents.
	 * @param type Wether to start all downloads or seeds. 1=Downloads, 2=Seeds, 3=All
	 */
	void stopAll(int type);

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
					 bool seed,const QString & output_file,bool priv_tor,KProgress* prog);

	CurrentStats getStats();

	/**
	 * Switch the port when no torrents are running.
	 * @param port The new port
	 * @return true if we can, false if there are torrents running
	 */
	bool changePort(bt::Uint16 port);

	///  Get the number of torrents running (including seeding torrents).
	bt::Uint32 getNumTorrentsRunning() const;

	///  Get the number of torrents not running.
	bt::Uint32 getNumTorrentsNotRunning() const;
	
	///Inserts blocked IP range into IPBlocklist
	void addBlockedIP(QString& ip);
	
	///Removes blocked IP range from IPBlocklist
	void removeBlockedIP(QString& ip);
	
	/**
	 * Find the next free torX dir.
	 * @return Path to the dir (including the torX part)
	 */
	QString findNewTorrentDir() const;
	
	/**
	 * Load plugins. 
	 */
	void loadPlugins();
	
	virtual void load(const KURL& url);
	virtual void loadSilently(const KURL& url);
	
	
	bt::QueueManager* getQueueManager();
	
	///Gets the number of torrents running
	int getNumRunning(bool onlyDownloads = true, bool onlySeeds = false) const;
	///Gets the number of torrents that are in state 'download' - total
	int countDownloads() const;
	///Gets the number of torrents that are in state 'seed' - total
	int countSeeds() const;
	
	int getMaxDownloadSpeed();
	int getMaxUploadSpeed();
	
	void setPausedState(bool pause);
	
	kt::TorrentInterface* getTorFromNumber(int tornumber);
	QValueList<int> getTorrentNumbers(int type);
        unsigned int getFileCount(int tornumber);
        QCStringList getFileNames(int tornumber);
        QValueList<int> getFilePriorities(int tornumber);
        void setFilePriority(kt::TorrentInterface* tc, bt::Uint32 index, int priority);
        void announceByTorNum(int tornumber);

public slots:
	/**
	 * Load a torrent file. Pops up an error dialog
	 * if something goes wrong.
	 * @param file The torrent file
	 * @param dir Directory to save the data
	 * @param silently Wether or not to do this silently
	 */
	void load(const QString & file,const QString & dir,bool silently);
	
	/**
	 * Remove a download.This will delete all temp
	 * data from this TorrentInterface And delete the
	 * TorrentInterface itself. It can also potentially
	 * start a new download (when one is waiting to be downloaded).
	 * @param tc
	 * @param data_to 
	 */
	void remove(kt::TorrentInterface* tc,bool data_to);

	/**
	 * Update all torrents.
	 */
	void update();
	
		/**
	 * Start a torrent, takes into account the maximum number of downloads.
	 * @param tc The TorrentInterface
		 */
	void start(kt::TorrentInterface* tc);

	/**
	 * Stop a torrent, may start another download if it hasn't been started.
	 * @param tc The TorrentInterface
	 */
	void stop(kt::TorrentInterface* tc, bool user = false);
	
	/**
	 * Enqueue/Dequeue function. Places a torrent in queue. 
	 * If the torrent is already in queue this will remove it from queue.
	 * @param tc TorrentControl pointer.
	 */
	void queue(kt::TorrentInterface* tc);
	
	/**
	 * A torrent is about to be started. We will do some file checks upon this signal.
	 * @param tc The TorrentInterface
	*/
	void aboutToBeStarted(kt::TorrentInterface* tc);
	
signals:
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
	
	/**
	* TorrentCore torrents have beed updated. Stats are changed.
	**/
	void statsUpdated();

private:
	void rollback(const QPtrList<kt::TorrentInterface> & success);
	void connectSignals(kt::TorrentInterface* tc);
	
private slots:
	void torrentFinished(kt::TorrentInterface* tc);
	void slotStoppedByError(kt::TorrentInterface* tc, QString msg);
	void torrentSeedAutoStopped(kt::TorrentInterface* tc);
	
private:
// 	QPtrList<kt::TorrentInterface> downloads;
	QString data_dir;
	int max_downloads;
	bool keep_seeding;
	QTimer update_timer;
	bt::Uint64 removed_bytes_up,removed_bytes_down;
	kt::PluginManager* pman;
	bt::QueueManager* qman;
};

#endif
