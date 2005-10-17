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

class KProgressDialog;

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
		 * @param torrent The filename of the torrent file
		 * @param datadir The directory to store the data
		 * @throw Error when something goes wrong
		 */
		void init(const QString & torrent,const QString & datadir);

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

		/// Get the data directory of this torrent
		QString getDataDir() const {return datadir;}

		/// Set the monitor
		void setMonitor(kt::MonitorInterface* tmo);

		

		/// Get the Torrent.
		const Torrent & getTorrent() const {return *tor;}

		/// Return an error message (only valid when status == ERROR).
		QString getErrorMessage() const {return error_msg;}
		
		/**
		 * Set the interval between two tracker updates.
		 * @param interval The interval in milliseconds
		 */
		void setTrackerTimerInterval(Uint32 interval);
		
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

		
		virtual Uint32 getNumFiles() const;
		virtual kt::TorrentFileInterface & getTorrentFile(Uint32 index);
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
		 * When the torrent is finished, the final file(s) can be
		 * reconstructed.
		 * @param dir The or directory to store files
		 */
		void reconstruct(const QString & dir);
		
		/**
		 * Update the tracker, this should normally handled internally.
		 * We leave it public so that the user can do a manual announce.
		 */
		void updateTracker() {updateTracker(QString::null);}

		/// Get the time to the next tracker update in milliseconds.
		Uint32 getTimeToNextTrackerUpdate() const;

		/// Get a short error message
		QString getShortErrorMessage() const {return short_error_msg;}
		
	private slots:
		void onNewPeer(Peer* p);
		void onPeerRemoved(Peer* p);
		void doChoking();

		/**
		 * An error occured during the update of the tracker.
		 */
		void trackerResponseError();

		/**
		 * The Tracker updated.
		 */
		void trackerResponse();
		
	private:	
		void updateTracker(const QString & ev,bool last_succes = true);
		void updateStatusMsg();
		void saveStats();
		void loadStats();
		void updateStats();
		void getSeederInfo(Uint32 & total,Uint32 & connected_to) const;
		void getLeecherInfo(Uint32 & total,Uint32 & connected_to) const;

		
	private:
		Torrent* tor;
		Tracker* tracker;
		ChunkManager* cman;
		PeerManager* pman;
		Downloader* down;
		Uploader* up;
		Choker* choke;
		
		Timer tracker_update_timer,choker_update_timer,stats_save_timer;
		Uint32 tracker_update_interval;
		
		QString datadir,old_datadir,trackerevent;
		QString error_msg,short_error_msg;
		Uint16 port;
		kt::MonitorInterface* tmon;
		KURL last_tracker_url;
		QDateTime time_started_dl, time_started_ul;
		unsigned long running_time_dl, running_time_ul;
		Uint64 prev_bytes_dl, prev_bytes_ul;
	};
}

#endif
