/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Ivan Vasic <ivasic@gmail.com>                                         *
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
#ifndef BTTORRENTCONTROL_H
#define BTTORRENTCONTROL_H

#include <qobject.h>
#include <qcstring.h> 
#include <qtimer.h>
#include <kurl.h>
#include "globals.h"
#include <util/timer.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/monitorinterface.h>
#include <interfaces/trackerslist.h>

class QStringList;
class QString;

namespace KIO
{
	class Job;
}


namespace bt
{
	class Choker;
	class Torrent;
	class PeerSourceManager;
	class ChunkManager;
	class PeerManager;
	class Downloader;
	class Uploader;
	class Peer;
	class BitSet;
	class QueueManager;
	class PreallocationThread;
	class TimeEstimator;
	class DataCheckerThread;
	class WaitJob;
	
	/**
	 * @author Joris Guisson
	 * @brief Controls just about everything
	 * 
	 * This is the interface which any user gets to deal with.
	 * This class controls the uploading, downloading, choking,
	 * updating the tracker and chunk management.
	 */
	class TorrentControl : public kt::TorrentInterface
	{
		Q_OBJECT
	public:
		TorrentControl();
		virtual ~TorrentControl();

		/**
		 * Get a BitSet of the status of all Chunks
		 */
		const BitSet & downloadedChunksBitSet() const;

		/**
		 * Get a BitSet of the availability of all Chunks
		 */
		const BitSet & availableChunksBitSet() const;

		/**
		 * Get a BitSet of the excluded Chunks
		 */
		const BitSet & excludedChunksBitSet() const;
		
		/**
		 * Get a BitSet of the only seed chunks 
		 */
		const BitSet & onlySeedChunksBitSet() const;
		
		/**
		 * Initialize the TorrentControl. 
		 * @param qman The QueueManager
		 * @param torrent The filename of the torrent file
		 * @param tmpdir The directory to store temporary data
		 * @param datadir The directory to store the actual file(s)
		 * 		(only used the first time we load a torrent)
		 * @param default_save_dir Default save directory (null if not set)
		 * @throw Error when something goes wrong
		 */
		void init(QueueManager* qman,
				const QString & torrent,
				const QString & tmpdir,
				const QString & datadir,
				const QString & default_save_dir);
		
		/**
		 * Initialize the TorrentControl. 
		 * @param qman The QueueManager
		 * @param data The data of the torrent
		 * @param tmpdir The directory to store temporary data
		 * @param datadir The directory to store the actual file(s)
		 * 		(only used the first time we load a torrent)
		 * @param default_save_dir Default save directory (null if not set)
		 * @throw Error when something goes wrong
		 */
		void init(QueueManager* qman,
				  const QByteArray & data,
				  const QString & tmpdir,
				  const QString & datadir,
				  const QString & default_save_dir);

		/**
		 * Change to a new data dir. If this fails
		 * we will fall back on the old directory.
		 * @param new_dir The new directory
		 * @return true upon succes
		 */
		bool changeDataDir(const QString & new_dir);
		
		
		/**
		 * Change torrents output directory. If this fails we will fall back on the old directory.
		 * @param new_dir The new directory
		 * @param moveFiles Wheather to actually move the files or just change the directory without moving them.
		 * @return true upon success.
		 */
		bool changeOutputDir(const QString& new_dir, bool moveFiles = true);

		/**
		 * Roll back the previous changeDataDir call.
		 * Does nothing if there was no previous changeDataDir call.
		 */
		void rollback();
	
		/// Gets the TrackersList interface
		kt::TrackersList* getTrackersList();
		
		/// Gets the TrackersList interface
		const kt::TrackersList* getTrackersList() const;
	
		/// Get the data directory of this torrent
		QString getDataDir() const {return outputdir;}

		/// Get the torX dir.
		QString getTorDir() const {return datadir;}

		/// Set the monitor
		void setMonitor(kt::MonitorInterface* tmo);

		/// Get the Torrent.
		const Torrent & getTorrent() const {return *tor;}
		
		/**
		 * Get the download running time of this torrent in seconds
		 * @return Uint32 - time in seconds
		 */
		Uint32 getRunningTimeDL() const;
		
		/**
		 * Get the upload running time of this torrent in seconds
		 * @return Uint32 - time in seconds
		 */
		Uint32 getRunningTimeUL() const;

		/**
		* Checks if torrent is multimedial and chunks needed for preview are downloaded
		* @param start_chunk The index of starting chunk to check
		* @param end_chunk The index of the last chunk to check
		* In case of single torrent file defaults can be used (0,1)
		**/
		bool readyForPreview(int start_chunk = 0, int end_chunk = 1);

		/// Get the time to the next tracker update in seconds.
		Uint32 getTimeToNextTrackerUpdate() const;

		/// Get a short error message
		QString getShortErrorMessage() const {return error_msg;}
		
		virtual Uint32 getNumFiles() const;
		virtual kt::TorrentFileInterface & getTorrentFile(Uint32 index);
		virtual void recreateMissingFiles();
		virtual void dndMissingFiles();
		virtual void addPeerSource(kt::PeerSource* ps);
		virtual void removePeerSource(kt::PeerSource* ps);
		
		int getPriority() const { return istats.priority; }
		void setPriority(int p);

		virtual bool overMaxRatio();		
		virtual void setMaxShareRatio(float ratio);
		virtual float getMaxShareRatio() const { return stats.max_share_ratio; }
		
		virtual bool overMaxSeedTime();
		virtual void setMaxSeedTime(float hours);
		virtual float getMaxSeedTime() const {return stats.max_seed_time;}
		
		/// Tell the TorrentControl obj to preallocate diskspace in the next update
		void setPreallocateDiskSpace(bool pa) {prealloc = pa;}
		
		/// Make a string out of the status message
		virtual QString statusToString() const;
		
		/// Checks if tracker announce is allowed (minimum interval 60 seconds)
		bool announceAllowed();
		
		void startDataCheck(bt::DataCheckerListener* lst,bool auto_import);
		
		/// Test if the torrent has existing files, only works the first time a torrent is loaded
		bool hasExistingFiles() const;
		
		/**
		 * Test all files and see if they are not missing.
		 * If so put them in a list
		 */
		bool hasMissingFiles(QStringList & sl);
		
		
		virtual Uint32 getNumDHTNodes() const;
		virtual const kt::DHTNode & getDHTNode(Uint32 i) const;
		virtual void deleteDataFiles();
		virtual const SHA1Hash & getInfoHash() const;
		virtual const bt::PeerID & getOwnPeerID() const;
		
		/**
		 * Called by the PeerSourceManager when it is going to start a new tracker.
		 */
		void resetTrackerStats();
		
		/**
		 * Returns estimated time left for finishing download. Returned value is in seconds.
		 * Uses TimeEstimator class to calculate this value.
		 */
		Uint32 getETA();

		/// Is a feature enabled
		bool isFeatureEnabled(kt::TorrentFeature tf);
		
		/// Disable or enable a feature
		void setFeatureEnabled(kt::TorrentFeature tf,bool on);
		
		/// Create all the necessary files
		void createFiles();
		
		///Checks if diskspace is low
		bool checkDiskSpace(bool emit_sig = true);
		
		virtual void setTrafficLimits(Uint32 up,Uint32 down);
		virtual void getTrafficLimits(Uint32 & up,Uint32 & down);
		
		///Get the PeerManager
		const PeerManager * getPeerMgr() const;
		
		/// Are we in the process of moving files
		bool isMovingFiles() const {return moving_files;}
		
	public slots:
		/**
		 * Update the object, should be called periodically.
		 */
		void update();
		
		/**
		 * Start the download of the torrent.
		 */
		void start();
		
		/**
		 * Stop the download, closes all connections.
		 * @param user wether or not the user did this explicitly
		 * @param wjob WaitJob to wait at exit for the completion of stopped requests
		 */
		void stop(bool user,WaitJob* wjob = 0);
			
		/**
		 * Update the tracker, this should normally handled internally.
		 * We leave it public so that the user can do a manual announce.
		 */
		void updateTracker();

		/**
		 * The tracker status has changed.
		 * @param ns New status
		 */
		void trackerStatusChanged(const QString & ns);
		
	private slots:
		void onNewPeer(Peer* p);
		void onPeerRemoved(Peer* p);
		void doChoking();
		void onIOError(const QString & msg);
		void onPortPacket(const QString & ip,Uint16 port);
		/// Update the stats of the torrent.
		void updateStats();
		void corrupted(Uint32 chunk);
		void moveDataFilesJobDone(KIO::Job* job);
		
	private:	
		void updateTracker(const QString & ev,bool last_succes = true);
		void updateStatusMsg();
		void saveStats();
		void loadStats();
		void loadOutputDir();
		void getSeederInfo(Uint32 & total,Uint32 & connected_to) const;
		void getLeecherInfo(Uint32 & total,Uint32 & connected_to) const;
		void migrateTorrent(const QString & default_save_dir);
		void continueStart();
		virtual void handleError(const QString & err);

		void initInternal(QueueManager* qman,const QString & tmpdir,
						  const QString & ddir,const QString & default_save_dir,bool first_time);
		
		void checkExisting(QueueManager* qman);
		void setupDirs(const QString & tmpdir,const QString & ddir);
		void setupStats();
		void setupData(const QString & ddir);
		virtual void afterDataCheck();
		virtual bool isCheckingData(bool & finished) const;
		
	private:
		Torrent* tor;
		PeerSourceManager* psman;
		ChunkManager* cman;
		PeerManager* pman;
		Downloader* down;
		Uploader* up;
		Choker* choke;
		TimeEstimator* m_eta;
		kt::MonitorInterface* tmon;
		
		Timer choker_update_timer;
		Timer stats_save_timer;
		Timer stalled_timer;
		
		QString datadir;
		QString old_datadir;
		QString outputdir;
		QString error_msg;
		
		QString move_data_files_destination_path;
		bool restart_torrent_after_move_data_files;
		
		bool prealloc;
		PreallocationThread* prealoc_thread;
		DataCheckerThread* dcheck_thread;
		TimeStamp last_diskspace_check;
		bool moving_files;
		
		struct InternalStats
		{
			QDateTime time_started_dl; 
			QDateTime time_started_ul;
			Uint32 running_time_dl;
			Uint32 running_time_ul;
			Uint64 prev_bytes_dl;
			Uint64 prev_bytes_ul;
			Uint64 trk_prev_bytes_dl;
			Uint64 trk_prev_bytes_ul;
			Uint64 session_bytes_uploaded;
			bool io_error;
			bool custom_output_name;
			Uint16 port;
			int priority;
			bool dht_on;
			TimeStamp last_announce;
			bool diskspace_warning_emitted;
		};
		
		Uint32 upload_gid; // group ID for upload
		Uint32 upload_limit; 
		Uint32 download_gid; // group ID for download
		Uint32 download_limit; 
		
		InternalStats istats;
	};
	
	
}

#endif
