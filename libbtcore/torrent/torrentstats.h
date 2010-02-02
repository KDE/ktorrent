/***************************************************************************
 *   Copyright (C) 2009 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
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

#ifndef BT_TORRENTSTATS_H
#define BT_TORRENTSTATS_H

#include <QString>
#include <util/constants.h>
#include <btcore_export.h>
#include <qdatetime.h>

#if defined ERROR
#undef ERROR
#endif

namespace bt 
{
	enum TorrentStatus
	{
		NOT_STARTED,
		SEEDING_COMPLETE,
		DOWNLOAD_COMPLETE,
		SEEDING,
		DOWNLOADING,
		STALLED,
		STOPPED,
		ALLOCATING_DISKSPACE,
		ERROR,
		QUEUED,
		CHECKING_DATA,
		NO_SPACE_LEFT,
		PAUSED,
		INVALID_STATUS
	};

	struct BTCORE_EXPORT TorrentStats
	{
		/// The number of bytes imported (igore these for average speed)
		Uint64 imported_bytes;
		/// Total number of bytes downloaded.
		Uint64 bytes_downloaded;
		/// Total number of bytes uploaded.
		Uint64 bytes_uploaded;
		/// The number of bytes left (gets sent to the tracker)
		Uint64 bytes_left;
		/// The number of bytes left to download (bytes_left - excluded bytes)
		Uint64 bytes_left_to_download;
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
		/// Get the number of chunks left
		Uint32 num_chunks_left;
		/// Size of each chunk
		Uint32 chunk_size;
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
		/// The number of bytes downloaded in this session
		Uint64 session_bytes_downloaded;
		/// The number of bytes uploaded in this session
		Uint64 session_bytes_uploaded;
		/// Name of the torrent
		QString torrent_name;
		/// Path of the dir or file where the data will get saved
		QString output_path;
		/// See if we are running
		bool running;
		/// See if the torrent has been started
		bool started;
		/// Whether or not the torrent is queued
		bool queued;
		/// See if we are allowed to startup this torrent automatically.
		bool autostart;
		/// See if we have a multi file torrent
		bool multi_file_torrent;
		/// See if the torrent is stopped by error
		bool stopped_by_error;
		/// See if the download is completed
		bool completed;
		/// Maximum share ratio
		float max_share_ratio;
		/// Maximum seed time in hours
		float max_seed_time;
		/// Private torrent (i.e. no use of DHT)
		bool priv_torrent;
		/// Number of corrupted chunks found since the last check
		Uint32 num_corrupted_chunks;
		/// TimeStamp when we last saw download activity
		TimeStamp last_download_activity_time;
		/// TimeStamp when we last saw upload activity
		TimeStamp last_upload_activity_time;
		/// Whether or not the QM can start this torrent
		bool qm_can_start;
		/// See if this torrent is paused
		bool paused;
		/// Error message for the user
		QString error_msg;
		/// QDateTime when the torrent was added
		QDateTime time_added;
		
		TorrentStats();
		
		/// Calculate the share ratio
		float shareRatio() const;
		
		/// Are we over the max share ratio
		bool overMaxRatio() const;
		
		/// Convert the status into a human readable string
		QString statusToString() const;
	};
}

#endif // BT_TORRENTSTATS_H
