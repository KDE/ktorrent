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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef BTDOWNLOADCAP_H
#define BTDOWNLOADCAP_H

#include <qptrlist.h>
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
		QPtrList<PeerDownloader> pdowners;
	
		DownloadCap();
	public:
		~DownloadCap();
		
		/**
		 * Set the speed cap in bytes per second. 0 indicates
		 * no limit.
		 * @param max Maximum number of bytes per second.
		*/
		void setMaxSpeed(Uint32 max);
		
		void update();
		
		void addPeerDonwloader(PeerDownloader* pd);
		void removePeerDownloader(PeerDownloader* pd);
		
		static DownloadCap & instance() {return self;}
	private:
		void capPD(PeerDownloader* pd,Uint32 cap);
		int numActiveDownloaders();
		void capAll(float max_speed_per_pd);
		void calcExcess(float max_speed_per_pd,float & exc_bw,int & num_maxed_out);
	};

}

#endif
