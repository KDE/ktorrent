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
#ifndef KTTORRENTINTERFACE_H
#define KTTORRENTINTERFACE_H

#include <qobject.h>
#include <util/constants.h>

class KURL;

namespace bt
{
	class BitSet;
}

namespace kt
{
	using bt::Uint32;
	using bt::Uint64;

	class MonitorInterface;
	class TorrentFileInterface;
	
	enum TorrentStatus
	{
		NOT_STARTED,
		COMPLETE,
		SEEDING,
		DOWNLOADING,
		STALLED,
		STOPPED,
		ERROR
	};

	struct TorrentStats
	{
		/// Total number of bytes downloaded.
		Uint64 bytes_downloaded;
		/// Total number of bytes uploaded.
		Uint64 bytes_uploaded;
		/// The number of bytes left to download
		Uint64 bytes_left;
		/// total number of bytes in torrent
		Uint64 total_bytes;
		/// The total number of bytes which need to be downloaded
		Uint64 total_bytes_to_download;
		/// The download rate in bytes per sec
		Uint32 download_rate;
		/// The upload rate in bytes per sec
		Uint32 upload_rate;
		/// The number of peers we are connected to
		Uint32 num_peers;
		/// The number of chunks we are currently downloading
		Uint32 num_chunks_downloading;
		/// The total number of chunks
		Uint32 total_chunks;
		/// The number of chunks which have been downloaded
		Uint32 num_chunks_downloaded;
		/// Get the number of chunks which have been excluded
		Uint32 num_chunks_excluded;
		/// Total seeders in swarm
		Uint32 seeders_total;
		/// Num seeders connected to
		Uint32 seeders_connected_to;
		/// Total leechers in swarm
		Uint32 leechers_total;
		/// Num leechers connected to
		Uint32 leechers_connected_to;
		/// Status of the download
		TorrentStatus status;
		/// The status of the tracker
		QString trackerstatus;
		/// The number of bytes downloaded in this session
		Uint64 session_bytes_downloaded;
		/// The number of bytes uploaded in this session
		Uint64 session_bytes_uploaded;
		/// Name of the torrent
		QString torrent_name;
		/// See if we are running
		bool running;
		/// See if the torrent has been started
		bool started;
		/// See if we are allowed to startup this torrent automatically.
		bool autostart;
		/// See if we have a multi file torrent
		bool multi_file_torrent;
		/// See if the torrent is stopped by error
		bool stopped_by_error;
		/// See if the download is completed
		bool completed;
	};

	/**
	 * @author Joris Guisson
	 * @brief Interface for an object which controls one torrent
	 *
	 * This class is the interface for an object which controls the
	 * up- and download of one torrent.
	*/
	class TorrentInterface : public QObject
	{
		Q_OBJECT
	public:
		TorrentInterface();
		virtual ~TorrentInterface();

		

		/**
		 * Update the object, should be called periodically.
		 */
		virtual void update() = 0;
		
		/**
		 * Start the download of the torrent.
		 */
		virtual void start() = 0;
		
		/**
		 * Stop the download, closes all connections.
		 * @param user wether or not the user did this explicitly
		 */
		virtual void stop(bool user) = 0;
		
		/**
		 * Update the tracker, this should normally handled internally.
		 * We leave it public so that the user can do a manual announce.
		 */
		virtual void updateTracker() = 0;

		/// Get the torrent's statistics
		const TorrentStats & getStats() const {return stats;}

		/**
		 * Checks if torrent is multimedial and chunks needed for preview are downloaded
		 * @param start_chunk The index of starting chunk to check
		 * @param end_chunk The index of the last chunk to check
		 * In case of single torrent file defaults can be used (0,1)
		 **/
		virtual bool readyForPreview(int start_chunk = 0, int end_chunk = 1) = 0;

		/**
		 * Get the torX directory of this torrent. Temporary stuff like the index
		 * file get stored there.
		 */
		virtual QString getTorDir() const = 0;

		/// Get the data directory of this torrent
		virtual QString getDataDir() const = 0;

		/// Get a short error message
		virtual QString getShortErrorMessage() const = 0;

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
		 * Change to a new data dir. If this fails
		 * we will fall back on the old directory.
		 * @param new_dir The new directory
		 * @return true upon succes
		 */
		virtual bool changeDataDir(const QString & new_dir) = 0;

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

		/// Set the monitor
		virtual void setMonitor(MonitorInterface* tmo) = 0;

		/// Get the time to the next tracker update in seconds.
		virtual Uint32 getTimeToNextTrackerUpdate() const = 0;

		/// Get the number of files in a multifile torrent (0 if we do not have a multifile torrent)
		virtual Uint32 getNumFiles() const = 0;

		/**
		 * Get the index'th file of a multifile torrent
		 * @param index The index
		 * @return The TorrentFileInterface (isNull() will be true in case of error)
		 */
		virtual TorrentFileInterface & getTorrentFile(Uint32 index) = 0;

		/**
		 * Get a tracker url. Used by the tracker to obtain an URL.
		 * @param prev_success Wether or not previous tracker update was succesfull
		 * @return An URL
		 */
		virtual KURL getTrackerURL(bool prev_success) const = 0;
		
		
		///Get the torrent queue number. Zero if not in queue
		virtual int getPriority() const = 0;
		
		///Set the torrent queue number.
		virtual void setPriority(int p) = 0;
		
	signals:
		/**
		 * Emited when we have finished downloading.
		 * @param me The object who emitted the signal
		 */
		void finished(kt::TorrentInterface* me);

		/**
		 * Emited when a Torrent download is stopped by error
		 * @param me The object who emitted the signal
		 * @param msg Error message
		 */
		void stoppedByError(kt::TorrentInterface* me, QString msg);

	protected:
		TorrentStats stats;
	};



}

#endif
