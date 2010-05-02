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

#ifndef BT_JOB_H
#define BT_JOB_H

#include <kio/job.h>
#include <btcore_export.h>
#include "torrentstats.h"

namespace bt 
{
	class TorrentControl;

	/**
		A Job is a KIO::Job which runs on a torrent
	*/
	class BTCORE_EXPORT Job : public KIO::Job
	{
		Q_OBJECT
	public:
		Job(bool stop_torrent,TorrentControl* tc);
		virtual ~Job();
		
		/// Do we need to stop the torrent when the job is running
		bool stopTorrent() const {return stop_torrent;}
		
		virtual void start();
		virtual void kill(bool quietly=true);
		
		/// Return the status of the torrent during the job (default implementation returns INVALID_STATUS)
		virtual TorrentStatus torrentStatus() const;
		
		TorrentControl* torrent() {return tc;}
	private:
		TorrentControl* tc;
		bool stop_torrent;
	};

}

#endif // BT_JOB_H
