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
#include <KLocale>
#include "torrentstats.h"
#include <util/functions.h>

namespace bt
{
	TorrentStats::TorrentStats()
	{
		imported_bytes = 0;
		running = false;
		started = false;
		queued = false;
		stopped_by_error = false;
		session_bytes_downloaded = 0;
		session_bytes_uploaded = 0;
		status = NOT_STARTED;
		autostart = false;
		priv_torrent = false;
		seeders_connected_to = seeders_total = 0;
		leechers_connected_to = leechers_total = 0;
		max_share_ratio = 0.00f;
		max_seed_time = 0;
		last_download_activity_time = last_upload_activity_time = bt::CurrentTime();
		num_corrupted_chunks = 0;
		qm_can_start = false;
		paused = false;
	}
	
	
	float TorrentStats::shareRatio() const
	{
		if (bytes_downloaded == 0)
			return 0.0f;
		else
			return (float) bytes_uploaded / (bytes_downloaded /*+ stats.imported_bytes*/);
	}
	
	
	bool TorrentStats::overMaxRatio() const
	{
		if (completed && max_share_ratio > 0)
			return shareRatio() >= max_share_ratio;
		else
			return false;
	}
	
	QString TorrentStats::statusToString() const
	{
		switch (status)
		{
			case NOT_STARTED :
				return i18n("Not started");
			case DOWNLOAD_COMPLETE :
				return i18n("Download completed");
			case SEEDING_COMPLETE :
				return i18n("Seeding completed");
			case SEEDING :
				return i18nc("Status of a torrent file", "Seeding");
			case DOWNLOADING:
				return i18n("Downloading");
			case STALLED:
				return i18n("Stalled");
			case STOPPED:
				return i18n("Stopped");
			case ERROR :
				return i18n("Error: %1",error_msg); 
			case ALLOCATING_DISKSPACE:
				return i18n("Allocating diskspace");
			case QUEUED:
				return completed ? i18n("Queued for seeding") : i18n("Queued for downloading");
			case CHECKING_DATA:
				return i18n("Checking data");
			case NO_SPACE_LEFT:
				return i18n("Stopped. No space left on device.");
			case PAUSED:
				return i18n("Paused");
			default:
				return QString();
		}
	}
}

