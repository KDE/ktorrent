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
#ifndef BTDOWNLOADCAP_H
#define BTDOWNLOADCAP_H

#include <qvaluelist.h>
#include <libutil/timer.h>
#include "globals.h"

namespace bt
{
	class PeerDownloader;

	/**
	 * @author Joris Guisson
	*/
	class DownloadCap
	{
		static DownloadCap self;

		Uint32 max_bytes_per_sec;
		double req_interval;
		Timer timer;
		QValueList<PeerDownloader*> dl_queue;
	
		DownloadCap();
	public:
		~DownloadCap();
		
		/**
		 * Set the speed cap in bytes per second. 0 indicates
		 * no limit.
		 * @param max Maximum number of bytes per second.
		*/
		void setMaxSpeed(Uint32 max);

		/**
		 * Allow or disallow somebody from requesting a piece. If somebody
		 * is disallowed they will be stored in a queue, and will be notified
		 * when there turn is up.
		 * @param pd PeerDownloader doing the request
		 * @return true if the piece is allowed or not
		*/
		bool allow(PeerDownloader* pd);

		/**
		 * PeerDownloader should call this when they get destroyed. To
		 * remove them from the queue.
		 * @param pd The PeerDownloader
		 */
		void killed(PeerDownloader* pd);

		/**
		 * Update the downloadcap.
		 */
		void update();
		
		static DownloadCap & instance() {return self;}
	};

}

#endif
