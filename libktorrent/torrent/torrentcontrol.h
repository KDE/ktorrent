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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
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


namespace bt
{
	class Choker;
	class Torrent;
	class Tracker;
	class ChunkManager;
	class PeerManager;
	class Downloader;
	class Uploader;
	class Peer;
	class BitSet;
	class QueueManager;
	
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
		 * Initialize the TorrentControl. 
		 * @param qman The QueueManager
		 * @param torrent The filename of the torrent file
		 * @param tmpdir The directory to store temporary data
		 * @param datadir The directory to store the actual file(s)
		 * 		(only used the first time we load a torrent)
		 * @param default_save_dir Default save directory (null if not set)
		 * @throw Error when something goes wrong
		 */
		void init(const QueueManager* qman,
				const QString & torrent,
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
		 * Roll back the previous changeDataDir call.
		 * Does nothing if there was no previous changeDataDir call.
		 */
		void rollback();

		KURL getTrackerURL(bool prev_success) const;
		
		/// Gets the TrackersList interface
		kt::TrackersList* getTrackersList();
		
		/// Creates TrackersList object (AnnounceList) and returns a pointer to that object
		kt::TrackersList* createTrackersList();

		/// Get the data directory of this torrent
		QString getDataDir() const {return outputdir;}

		/// Get the torX dir.
		QString getTorDir() const {return datadir;}

		/// Set the monitor
		void setMonitor(kt::MonitorInterface* tmo);

		/// Get the Torrent.
		const Torrent & getTorrent() const {return *tor;}

		/// Return an error message (only valid when status == ERROR).
		QString getErrorMessage() const {return error_msg;}
		
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
		QString getShortErrorMessage() const {return short_error_msg;}
		
		virtual Uint32 getNumFiles() const;
		virtual kt::TorrentFileInterface & getTorrentFile(Uint32 index);
		virtual void recreateMissingFiles();
		virtual void dndMissingFiles();
		
		int getPriority() const { return priority; }
		void setPriority(int p);

		bool overMaxRatio();		
		void setMaxShareRatio(float ratio);
		float getMaxShareRatio() const { return maxShareRatio; }
		
		/// Tell the TorrentControl obj to preallocate diskspace in the next update
		void setPreallocateDiskSpace(bool pa) {prealloc = pa;}
		
		/// Make a string out of the status message
		virtual QString statusToString() const;
		
		/// Checks if tracker announce is allowed (minimum interval 60 seconds)
		bool announceAllowed();
		
		void doDataCheck(bt::DataCheckerListener* lst);
		
		/// Test if the torrent has existing files, only works the first time a torrent is loaded
		bool hasExistingFiles() const;
		
		/**
		 * Test all files and see if they are not missing.
		 * If so put them in a list
		 */
		bool hasMissingFiles(QStringList & sl);
		
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
		 */
		void stop(bool user);
			
		/**
		 * Update the tracker, this should normally handled internally.
		 * We leave it public so that the user can do a manual announce.
		 */
		void updateTracker();

	
		
	private slots:
		void onNewPeer(Peer* p);
		void onPeerRemoved(Peer* p);
		void doChoking();
		void onIOError(const QString & msg);
		void onPortPacket(const QString & ip,Uint16 port);

		/**
		 * An error occured during the update of the tracker.
		 */
		void trackerResponseError();

		/**
		 * The Tracker updated.
		 */
		void trackerResponse();
		
		/// Update the stats of the torrent.
		void updateStats();
		
	private:	
		void updateTracker(const QString & ev,bool last_succes = true);
		void updateStatusMsg();
		void saveStats();
		void loadStats();
		void loadOutputDir();
		void getSeederInfo(Uint32 & total,Uint32 & connected_to) const;
		void getLeecherInfo(Uint32 & total,Uint32 & connected_to) const;
		void migrateTorrent(const QString & default_save_dir);

		
	private:
		Torrent* tor;
		Tracker* tracker;
		ChunkManager* cman;
		PeerManager* pman;
		Downloader* down;
		Uploader* up;
		Choker* choke;
		
		Timer choker_update_timer,stats_save_timer,stalled_timer;
		
		QString datadir,old_datadir,outputdir;
		QString error_msg,short_error_msg;
		Uint16 port;
		kt::MonitorInterface* tmon;
		QDateTime time_started_dl, time_started_ul;
		unsigned long running_time_dl, running_time_ul;
		Uint64 prev_bytes_dl, prev_bytes_ul;
		Uint64 trk_prev_bytes_dl, trk_prev_bytes_ul;
		Uint64 session_bytes_uploaded;
		bool io_error;
		bool custom_output_name;
		int priority;
		float maxShareRatio;
		bool prealloc;
		Uint32 last_announce;
	};
}

#endif
