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

#include <qdatetime.h>
#include <qobject.h>
#include <qtimer.h>
#include <kurl.h>
#include <util/timer.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/trackerslist.h>
#include <btcore_export.h>
#include "torrent.h"
#include "globals.h"

class QStringList;
class QString;
class KJob;


namespace bt
{
	class StatsFile;
	class Choker;
	class PeerSourceManager;
	class ChunkManager;
	class PeerManager;
	class Downloader;
	class Uploader;
	class Peer;
	class BitSet;
	class QueueManagerInterface;
	class TimeEstimator;
	class WaitJob;
	class MonitorInterface;
	class ChunkSelectorFactoryInterface;
	class CacheFactory;
	class JobQueue;
	
	/**
	 * @author Joris Guisson
	 * @brief Controls just about everything
	 * 
	 * This is the interface which any user gets to deal with.
	 * This class controls the uploading, downloading, choking,
	 * updating the tracker and chunk management.
	 */
	class BTCORE_EXPORT TorrentControl : public TorrentInterface, public FilePriorityListener
	{
		Q_OBJECT
	public:
		TorrentControl();
		virtual ~TorrentControl();
		
		/// Get the Torrent.
		const Torrent & getTorrent() const {return *tor;}
		
		/**
		 * Initialize the TorrentControl. 
		 * @param qman The QueueManager
		 * @param torrent The filename of the torrent file
		 * @param tmpdir The directory to store temporary data
		 * @param datadir The directory to store the actual file(s)
		 * 		(only used the first time we load a torrent)
		 * @throw Error when something goes wrong
		 */
		void init(QueueManagerInterface* qman,
				const QString & torrent,
				const QString & tmpdir,
				const QString & datadir);
		
		/**
		 * Initialize the TorrentControl. 
		 * @param qman The QueueManager
		 * @param data The data of the torrent
		 * @param tmpdir The directory to store temporary data
		 * @param datadir The directory to store the actual file(s)
		 * 		(only used the first time we load a torrent)
		 * @throw Error when something goes wrong
		 */
		void init(QueueManagerInterface* qman,
				  const QByteArray & data,
				  const QString & tmpdir,
				  const QString & datadir);
				  
				  
		/// Tell the TorrentControl obj to preallocate diskspace in the next update
		void setPreallocateDiskSpace(bool pa) {prealloc = pa;}
		
		/// Test if the torrent has existing files, only works the first time a torrent is loaded
		bool hasExistingFiles() const;

		virtual const BitSet & downloadedChunksBitSet() const;
		virtual const BitSet & availableChunksBitSet() const;
		virtual const BitSet & excludedChunksBitSet() const;
		virtual const BitSet & onlySeedChunksBitSet() const;
		virtual bool changeTorDir(const QString & new_dir);
		virtual bool changeOutputDir(const QString& new_dir,int flags);
		virtual void rollback();
		virtual void setDisplayName(const QString & n);
		virtual TrackersList* getTrackersList();
		virtual const TrackersList* getTrackersList() const;
		virtual QString getDataDir() const {return outputdir;}
		virtual QString getTorDir() const {return tordir;}
		virtual void setMonitor(MonitorInterface* tmo);
		virtual Uint32 getRunningTimeDL() const;
		virtual Uint32 getRunningTimeUL() const;
		virtual Uint32 getNumFiles() const;
		virtual TorrentFileInterface & getTorrentFile(Uint32 index);
		virtual const TorrentFileInterface & getTorrentFile(Uint32 index) const;
		virtual bool moveTorrentFiles(const QMap<TorrentFileInterface*,QString> & files);
		virtual void recreateMissingFiles();
		virtual void dndMissingFiles();
		virtual void addPeerSource(PeerSource* ps);
		virtual void removePeerSource(PeerSource* ps);
		virtual const QTextCodec* getTextCodec() const;
		virtual void changeTextCodec(QTextCodec* tc);
		virtual Uint32 getNumWebSeeds() const;
		virtual const WebSeedInterface* getWebSeed(Uint32 i) const;
		virtual WebSeedInterface* getWebSeed(Uint32 i);
		virtual bool addWebSeed(const KUrl & url);
		virtual bool removeWebSeed(const KUrl & url);
		virtual bool readyForPreview() const;
		virtual bool isMultimedia() const;
		virtual void markExistingFilesAsDownloaded();
		virtual int getPriority() const { return istats.priority; }
		virtual void setPriority(int p);
		virtual bool overMaxRatio();		
		virtual void setMaxShareRatio(float ratio);
		virtual float getMaxShareRatio() const { return stats.max_share_ratio; }
		virtual bool overMaxSeedTime();
		virtual void setMaxSeedTime(float hours);
		virtual float getMaxSeedTime() const {return stats.max_seed_time;}
		virtual void setAllowedToStart(bool on);
		virtual void setQueued(bool queued);
		virtual void setChunkSelector(ChunkSelectorInterface* csel);
		virtual void networkUp();
		virtual bool announceAllowed();
		virtual void startDataCheck(bt::DataCheckerListener* lst);
		virtual bool hasMissingFiles(QStringList & sl);
		virtual Uint32 getNumDHTNodes() const;
		virtual const DHTNode & getDHTNode(Uint32 i) const;
		virtual void deleteDataFiles();
		virtual const bt::PeerID & getOwnPeerID() const;
		virtual QString getComments() const;
		virtual const JobQueue* getJobQueue() const {return job_queue;}
		virtual bool isFeatureEnabled(TorrentFeature tf);
		virtual void setFeatureEnabled(TorrentFeature tf,bool on);
		virtual bool checkDiskSpace(bool emit_sig = true);
		virtual void setTrafficLimits(Uint32 up,Uint32 down);
		virtual void getTrafficLimits(Uint32 & up,Uint32 & down);
		virtual void setAssuredSpeeds(Uint32 up,Uint32 down);
		virtual void getAssuredSpeeds(Uint32 & up,Uint32 & down);
		virtual const SHA1Hash & getInfoHash() const;
		virtual void setUserModifiedFileName(const QString & n);
		virtual int getETA();
		
		/// Create all the necessary files
		void createFiles();
		
		/// Get the PeerManager
		const PeerManager * getPeerMgr() const;
		
		/// Set a custom chunk selector factory (needs to be done for init is called)
		void setChunkSelectorFactory(ChunkSelectorFactoryInterface* csfi);
		
		/// Set a custom Cache factory
		void setCacheFactory(CacheFactory* cf);
		
	public slots:
		/**
		 * Update the object, should be called periodically.
		 */
		void update();
		
		/**
		 * Pause the torrent.
		 */
		void pause();
		
		/**
		 * Unpause the torrent.
		 */
		void unpause();
		
		/**
		 * Start the download of the torrent.
		 */
		void start();
		
		/**
		 * Stop the download, closes all connections.
		 * @param wjob WaitJob to wait at exit for the completion of stopped requests
		 */
		void stop(WaitJob* wjob = 0);
			
		/**
		 * Update the tracker, this should normally handled internally.
		 * We leave it public so that the user can do a manual announce.
		 */
		void updateTracker();

		/**
		 * Scrape the tracker.
		 * */
		void scrapeTracker();

		/**
		 * A scrape has finished on the tracker.
		 * */
		void trackerScrapeDone();
		
		/**
		 * Set the move upon completion directory. 
		 * @param dir The directory an empty url disables this feature
		 */
		static void setMoveWhenCompletedDir(const KUrl & dir) {completed_dir = dir;}
		
		/**
		 * Enable or disable data check upon completion
		 * @param on 
		 */
		static void setDataCheckWhenCompleted(bool on) {completed_datacheck = on;}
		
		/**
		 * Set the minimum amount of diskspace in MB. When there is less then this free, torrents will be stopped.
		 * @param m 
		 */
		static void setMinimumDiskSpace(Uint32 m) {min_diskspace = m;}
		
		/**
		 * Enable or disable automatic datachecking when to many corrupted chunks have been found on disk.
		 * @param on 
		 */
		static void setAutoRecheck(bool on) {auto_recheck = on;}
		
		/**
		 * Set the number of corrupted chunks for a before we start an automatic recheck.
		 * @param m 
		 */
		static void setNumCorruptedForRecheck(Uint32 m) {num_corrupted_for_recheck = m;}
		
	protected:
		/// Called when a data check is finished by DataCheckerJob
		void afterDataCheck(DataCheckerListener* lst,const BitSet & result,const QString & error);
		void beforeDataCheck();
		void preallocFinished(const QString & error,bool completed);
		void allJobsDone();
		bool preallocate();
		
	private slots:
		void onNewPeer(Peer* p);
		void onPeerRemoved(Peer* p);
		void doChoking();
		void onIOError(const QString & msg);
		/// Update the stats of the torrent.
		void updateStats();
		void corrupted(Uint32 chunk);
		void moveDataFilesFinished(KJob* j);
		void moveDataFilesWithMapFinished(KJob* j);
		void downloaded(Uint32 chunk);
		void moveToCompletedDir();
		
	private:	
		void updateTracker(const QString & ev,bool last_succes = true);
		void updateStatus();
		void saveStats();
		void loadStats();
		void loadOutputDir();
		void loadEncoding();
		void getSeederInfo(Uint32 & total,Uint32 & connected_to) const;
		void getLeecherInfo(Uint32 & total,Uint32 & connected_to) const;
		void continueStart();
		virtual void handleError(const QString & err);
		void initInternal(QueueManagerInterface* qman,const QString & tmpdir,const QString & ddir);
		void checkExisting(QueueManagerInterface* qman);
		void setupDirs(const QString & tmpdir,const QString & ddir);
		void setupStats();
		void setupData();
		void setUploadProps(Uint32 limit,Uint32 rate);
		void setDownloadProps(Uint32 limit,Uint32 rate);
		virtual void downloadPriorityChanged(TorrentFile* tf, Priority newpriority, Priority oldpriority);
		
	signals:
		void dataCheckFinished();
		
	private:
		JobQueue* job_queue;
		Torrent* tor;
		PeerSourceManager* psman;
		ChunkManager* cman;
		PeerManager* pman;
		Downloader* downloader;
		Uploader* uploader;
		Choker* choke;
		TimeEstimator* m_eta;
		MonitorInterface* tmon;
		ChunkSelectorFactoryInterface* custom_selector_factory;
		CacheFactory* cache_factory;
		QString move_data_files_destination_path;
		Timer choker_update_timer;
		Timer stats_save_timer;
		Timer stalled_timer;
		Timer wanted_update_timer;
		QString tordir;
		QString old_tordir;
		QString outputdir;
		QString error_msg;
		bool prealloc;
		TimeStamp last_diskspace_check;
		
		struct InternalStats
		{
			QDateTime time_started_dl; 
			QDateTime time_started_ul;
			Uint32 running_time_dl;
			Uint32 running_time_ul;
			Uint64 prev_bytes_dl;
			Uint64 prev_bytes_ul;
			Uint64 session_bytes_uploaded;
			bool io_error;
			bool custom_output_name;
			Uint16 port;
			int priority;
			bool dht_on;
			bool diskspace_warning_emitted;
		};
		
		Uint32 upload_gid; // group ID for upload
		Uint32 upload_limit; 
		Uint32 download_gid; // group ID for download
		Uint32 download_limit; 
		
		Uint32 assured_download_speed;
		Uint32 assured_upload_speed;
		
		InternalStats istats;
		StatsFile* stats_file;
		
		static KUrl completed_dir;
		static bool completed_datacheck;
		static Uint32 min_diskspace;
		static bool auto_recheck;
		static Uint32 num_corrupted_for_recheck;
		
		friend class DataCheckerJob;
		friend class PreallocationJob;
		friend class JobQueue;
	};
	
	
}

#endif
