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
#ifndef BTTORRENTINTERFACE_H
#define BTTORRENTINTERFACE_H

#include <qobject.h>
#include <btcore_export.h>
#include <util/constants.h>
#include <interfaces/trackerslist.h>
#include <torrent/torrentstats.h>
#include <kurl.h>

#ifdef ERROR
#undef ERROR
#endif
namespace bt
{
	class BitSet;
	class DataCheckerListener;
	class SHA1Hash;
	class WaitJob;
	class PeerID;
	class MonitorInterface;
	class TorrentFileInterface;
	class PeerSource;
	class SHA1Hash;
	class WebSeedInterface;
	class JobQueue;
	class ChunkSelectorInterface;

	
	enum TorrentStartResponse
	{
		START_OK,
		USER_CANCELED,
		NOT_ENOUGH_DISKSPACE, 
		MAX_SHARE_RATIO_REACHED, 
		BUSY_WITH_JOB, 
		QM_LIMITS_REACHED // Max seeds or downloads reached
	};

	enum AutoStopReason
	{
		MAX_RATIO_REACHED,
		MAX_SEED_TIME_REACHED
	};
	

	struct DHTNode
	{
		QString ip;
		bt::Uint16 port;
	};
	
	enum TorrentFeature
	{
		DHT_FEATURE, 
		UT_PEX_FEATURE // µTorrent peer exchange
	};
	

	/**
	 * @author Joris Guisson
	 * @brief Interface for an object which controls one torrent
	 *
	 * This class is the interface for an object which controls the
	 * up- and download of one torrent.
	*/
	class BTCORE_EXPORT TorrentInterface : public QObject
	{
		Q_OBJECT
	public:
		TorrentInterface();
		virtual ~TorrentInterface();

		/// Set the URL which the torrent was loaded from
		void setLoadUrl(const KUrl& u) {url = u;}
		
		/// Get the URL which the torrent was loaded from
		KUrl loadUrl() const {return url;}
		
		/**
		 * Update the object, should be called periodically.
		 */
		virtual void update() = 0;
		
		/**
		 * Pause the torrent.
		 */
		virtual void pause() = 0;
		
		/**
		 * Unpause the torrent.
		 */
		virtual void unpause() = 0;
		
		/**
		 * Start the download of the torrent.
		 */
		virtual void start() = 0;
		
		/**
		 * Stop the download, closes all connections.
		 * @param wjob WaitJob, used when KT is shutting down, 
		 * so that we can wait for all stopped events to reach the tracker
		 */
		virtual void stop(bt::WaitJob* wjob = 0) = 0;
		
		/**
		 * Update the tracker, this should normally handled internally.
		 * We leave it public so that the user can do a manual announce.
		 */
		virtual void updateTracker() = 0;
		
		/**
		 * Scrape all or one tracker (private torrents)
		 */
		virtual void scrapeTracker() = 0;

		/// Get the torrent's statistics
		const TorrentStats & getStats() const {return stats;}

		/**
		 * Checks if torrent is multimedial and chunks needed for preview are downloaded
		 * This only works for single file torrents
		 * @return true if it is
		 **/
		virtual bool readyForPreview() const = 0;
		
		/// See if this is a single file torrent and a multimedia files
		virtual bool isMultimedia() const = 0;

		/**
		 * Get the torX directory of this torrent. Temporary stuff like the index
		 * file get stored there.
		 */
		virtual QString getTorDir() const = 0;

		/// Get the data directory of this torrent
		virtual QString getDataDir() const = 0;

		/**
		 * Get the download running time of this torrent in seconds
		 * @return Uint32 - time in seconds
		 */
		virtual Uint32 getRunningTimeDL() const = 0;
		
		/**
		 * Get the upload running time of this torrent in seconds
		 * @return Uint32 - time in seconds
		 */
		virtual Uint32 getRunningTimeUL() const = 0;

		/**
		 * Change to a new torX dir. If this fails
		 * we will fall back on the old directory.
		 * @param new_dir The new directory
		 * @return true upon succes
		 */
		virtual bool changeTorDir(const QString & new_dir) = 0;
		
		enum ChangeOutputFlags
		{
			MOVE_FILES = 1,FULL_PATH = 2
		};
		
		/**
		 * Change to a new data dir. If this fails
		 * we will fall back on the old directory.
		 * @param new_dir The new directory
		 * @param flags The OR of ChangeOutputFlags
		 * @return true upon succes
		 */
		virtual bool changeOutputDir(const QString& new_dir,int flags) = 0;

		/**
		 * Roll back the previous changeDataDir call.
		 * Does nothing if there was no previous changeDataDir call.
		 */
		virtual void rollback() = 0;

		/**
		 * Get a BitSet of the status of all Chunks
		 */
		virtual const bt::BitSet & downloadedChunksBitSet() const = 0;

		/**
		 * Get a BitSet of the availability of all Chunks
		 */
		virtual const bt::BitSet & availableChunksBitSet() const = 0;

		/**
		 * Get a BitSet of the excluded Chunks
		 */
		virtual const bt::BitSet & excludedChunksBitSet() const = 0;
		
		/**
		 * Get a bitset of only seed chunks
		 */
		virtual const bt::BitSet & onlySeedChunksBitSet() const = 0;

		/// Set the monitor
		virtual void setMonitor(MonitorInterface* tmo) = 0;

		/// Get the number of files in a multifile torrent (0 if we do not have a multifile torrent)
		virtual Uint32 getNumFiles() const = 0;

		/**
		 * Get the index'th file of a multifile torrent
		 * @param index The index
		 * @return The TorrentFileInterface (isNull() will be true in case of error)
		 */
		virtual TorrentFileInterface & getTorrentFile(Uint32 index) = 0;
		
		/**
		 * Const version of the previous one.
		 * @param index The index of the file
		 * @return The TorrentFileInterface (isNull() will be true in case of error)
		 */
		virtual const TorrentFileInterface & getTorrentFile(Uint32 index) const = 0;
		
		/**
		 * Move a torrent file to a new location.
		 * @param files Map of files and their new location
		 * @return true upon success
		 */
		virtual bool moveTorrentFiles(const QMap<TorrentFileInterface*,QString> & files) = 0;
		
		///Get a pointer to TrackersList object
		virtual TrackersList* getTrackersList() = 0;
		
		///Get a pointer to TrackersList object
		virtual const TrackersList* getTrackersList() const = 0;
		
		///Get the torrent queue number. Zero if not in queue
		virtual int getPriority() const = 0;
		
		///Set the torrent queue number.
		virtual void setPriority(int p) = 0;
		
		/// Set the max share ratio
		virtual void setMaxShareRatio(float ratio) = 0;
		
		/// Get the max share ratio
		virtual float getMaxShareRatio() const = 0;
		
		/// Set the max seed time in hours (0 is no limit)
		virtual void setMaxSeedTime(float hours) = 0;
		
		/// Get the max seed time
		virtual float getMaxSeedTime() const = 0;
		
		/// Get the comments
		virtual QString getComments() const = 0;
		
		/// Update the status
		virtual void updateStatus() = 0;
		
		///Is manual announce allowed?
		virtual bool announceAllowed() = 0;
		
		
		/**
		 * Returns estimated time left for finishing download. Returned value is in seconds.
		 * Uses TimeEstimator class to calculate this value.
		 */
		virtual int getETA() = 0;
		
		/**
		 * Verify the correctness of all data.
		 * @param lst The listener
		 */
		virtual void startDataCheck(bt::DataCheckerListener* lst) = 0;
		
		/**
		 * Test all files and see if they are not missing.
		 * If so put them in a list
		 */
		virtual bool hasMissingFiles(QStringList & sl) = 0;
		
		/**
		 * Recreate missing files.
		*/
		virtual void recreateMissingFiles() = 0;
		
		/**
		 * Mark missing files as do not download.
		 */
		virtual void dndMissingFiles() = 0;
		
		
		/// Get the number of initial DHT nodes
		virtual Uint32 getNumDHTNodes() const = 0;
		
		/// Get a DHT node
		virtual const DHTNode & getDHTNode(Uint32 i) const = 0;
	
		/** Delete the data files of the torrent,
		 * they will be lost permanently
		 */
		virtual void deleteDataFiles() = 0;
		
		///Checks if a seeding torrent has reached its maximum share ratio
		virtual bool overMaxRatio() = 0;
		
		/// Checks if a seeding torrent has reached it's max seed timery / will be ret
		virtual bool overMaxSeedTime() = 0;

		/// Handle an error
		virtual void handleError(const QString & err) = 0;
		
		/// Get the info_hash.
		virtual const bt::SHA1Hash & getInfoHash() const  = 0;
		
		/**
		 * Add a new PeerSource
		 * @param ps 
		 */
		virtual void addPeerSource(PeerSource* ps) = 0;
		
		/**
		 * Remove a nPeerSource
		 * @param ps 
		 */
		virtual void removePeerSource(PeerSource* ps) = 0;
		
		/// Is a feature enabled
		virtual bool isFeatureEnabled(TorrentFeature tf) = 0;
		
		/// Disable or enable a feature
		virtual void setFeatureEnabled(TorrentFeature tf,bool on) = 0;
		
		/// Get our PeerID
		virtual const bt::PeerID & getOwnPeerID() const = 0;
		
		/// Set the traffic limits for this torrent
		virtual void setTrafficLimits(Uint32 up,Uint32 down) = 0;
		
		/// Get the traffic limits
		virtual void getTrafficLimits(Uint32 & up,Uint32 & down) = 0;
		
		/// Set the assured speeds
		virtual void setAssuredSpeeds(Uint32 up,Uint32 down) = 0;
		
		/// Get the assured speeds
		virtual void getAssuredSpeeds(Uint32 & up,Uint32 & down) = 0;
		
		/// Check if there is enough diskspace available for this torrent
		virtual bool checkDiskSpace(bool emit_sig = true) = 0;
		
		/// Get the text codec used in the torrent
		virtual const QTextCodec* getTextCodec() const = 0;
		
		/// Set the text codec
		virtual void changeTextCodec(QTextCodec* tc) = 0;
		
		/// Get the number of webseeds
		virtual Uint32 getNumWebSeeds() const = 0;
		
		/// Get a webseed (returns 0 if index is invalid)
		virtual const WebSeedInterface* getWebSeed(Uint32 i) const = 0; 
		
		/// Get a webseed (returns 0 if index is invalid)
		virtual WebSeedInterface* getWebSeed(Uint32 i) = 0; 
		
		/// Add a webseed (return false, if there is already a webseed with the same url)
		virtual bool addWebSeed(const KUrl & url) = 0;
		
		/// Remove a webseed (only user created ones can be removed)
		virtual bool removeWebSeed(const KUrl & url) = 0;
		
		/// Mark all existing files as downloaded (
		virtual void markExistingFilesAsDownloaded() = 0;
		
		/// Set the user modified file or toplevel directory name
		virtual void setUserModifiedFileName(const QString & n) {user_modified_name = n;}
		
		/// Gets the user modified file or toplevel directory name
		QString getUserModifiedFileName() const {return user_modified_name.isEmpty() ? stats.torrent_name : user_modified_name;}

		/// Set the displayed name
		virtual void setDisplayName(const QString & n) = 0;
		
		/// Gets the displayed name
		QString getDisplayName() const {return display_name.isEmpty() ? stats.torrent_name : display_name;}
		
		/// Set whether the QM can start a torrent.
		virtual void setAllowedToStart(bool on) = 0;
		
		/**
		 * Can the torrent be started by the QM.
		 * @return True if it can, false otherwise
		 */
		bool isAllowedToStart() const {return stats.qm_can_start;}
		
		/**
		 * Set whether the torrent is queued or not
		 */
		virtual void setQueued(bool queued) = 0;
		
		/// Get the JobQueue of the torrent
		virtual const JobQueue* getJobQueue() const = 0;
		
		/// Set the ChunkSelector to use (0 resets to the default ChunkSelector)
		virtual void setChunkSelector(ChunkSelectorInterface* csel) = 0;
		
		/// After some network down time, the network is back up
		virtual void networkUp() = 0;
	signals:
		/**
		 * Emitted when we have finished downloading.
		 * @param me The object who emitted the signal
		 */
		void finished(bt::TorrentInterface* me);

		/**
		 * Emitted when a Torrent download is stopped by error
		 * @param me The object who emitted the signal
		 * @param msg Error message
		 */
		void stoppedByError(bt::TorrentInterface* me, QString msg);
		
		/**
		 * Emitted when maximum share ratio for this torrent is changed
		 * @param me The object which emitted the signal.
		 */
		void maxRatioChanged(bt::TorrentInterface* me);
		
		/**
		 * Emitted then torrent is stopped from seeding by KTorrent. 
		 * Happens when torrent has reached maximum share ratio and maybe we'll add something more...
		 * @param me The object which emitted the signal.
		 * @param reason The reason why it was aut stopped
		 */
		void seedingAutoStopped(bt::TorrentInterface* me,bt::AutoStopReason reason);
		
		/**
		 * Emitted just before the torrent is started, this should be used to do some
		 * checks on the files in the cache.
		 * @param me The torrent which emitted the signal
		 * @param ret The return value
		 */
		void aboutToBeStarted(bt::TorrentInterface* me,bool & ret);
		
		/**
		 * Emitted when missing files have been marked as dnd.
		 * The intention of this signal is to update the GUI.
		 * @param me The torrent which emitted the signal
		*/
		void missingFilesMarkedDND(bt::TorrentInterface* me);
		
		/**
		 * A corrupted chunk has been found during upload.
		 * @param me The torrent which emitted the signal
		 */
		void corruptedDataFound(bt::TorrentInterface* me);
		
		/**
		 * Disk is running out of space.
		 * @param me The torrent which emitted the signal
		 * @param toStop should this torrent be stopped or not
		 */
		void diskSpaceLow(bt::TorrentInterface* me, bool toStop);

		/**
		 * Torrent has been stopped
		 * @param me The torrent which emitted the signal
		 */
		void torrentStopped(bt::TorrentInterface* me);

		/**
		 * Signal emitted when the torrent needs a data check
		 * @param me The torrent
		 * */
		void needDataCheck(bt::TorrentInterface* me);
		
		/**
		 * Emitted whenever the status of the torrent changes.
		 * @param me the torrent
		 */
		void statusChanged(bt::TorrentInterface* me);
		
		/**
		 * Emitted when a chunk is downloaded.
		 * @param me The torrent
		 * @param chunk The chunk
		 */
		void chunkDownloaded(bt::TorrentInterface* me,bt::Uint32 chunk);
		
		/**
		 * Emitted when the torrent thinks the QM should update the queue
		 */
		void updateQueue();
		
		/**
		 * Emitted when all running jobs are done.
		 * @param me the torrent
		 */
		void runningJobsDone(bt::TorrentInterface* me);
		
	protected:
		TorrentStats stats;
		QString user_modified_name;
		QString display_name;
		KUrl url;
	};

}

#endif
