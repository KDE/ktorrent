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
#ifndef BTUPLOADCAP_H
#define BTUPLOADCAP_H

#include <qmap.h>
#include <qvaluelist.h>
#include <util/timer.h>
#include "globals.h"

namespace bt
{
	class PacketWriter;

	/**
	 * @author Joris Guisson
	 * @brief Keeps the upload rate under control
	 * 
	 * Before a PeerUploader can send a piece, it must first ask
	 * permission to a UploadCap object. This object will make sure
	 * that the upload rate remains under a specified threshold. When the
	 * threshold is set to 0, no upload capping will be done.
	*/
	class UploadCap
	{
		static UploadCap self;
		
		struct Entry
		{
			PacketWriter* pw;
			Uint32 bytes;
		};
		QValueList<Entry> up_queue;
		Uint32 max_bytes_per_sec;
		Timer timer;
		Uint32 leftover;

		UploadCap();
	public:
		~UploadCap();
		/**
		 * Set the speed cap in bytes per second. 0 indicates
		 * no limit.
		 * @param max Maximum number of bytes per second.
		 */
		void setMaxSpeed(Uint32 max);
		
		/// Get max bytes/sec
		Uint32 getMaxSpeed() const {return max_bytes_per_sec;}

		/**
		 * Allow or disallow somebody from sending a piece. If somebody
		 * is disallowed they will be stored in a queue, and will be notified
		 * when there turn is up.
		 * @param pd PacketWriter doing the request
		 * @param bytes Bytes it wants to send
		 * @return true if the piece is allowed or not
		 */
		bool allow(PacketWriter* pd,Uint32 bytes);

		/**
		 * PacketWriter should call this when they get destroyed. To
		 * remove them from the queue.
		 * @param pd The PeerUploader
		 */
		void killed(PacketWriter* pd);

		/**
		 * Update the downloadcap.
		 */
		void update();
	

		static UploadCap & instance() {return self;}
	};

}

#endif
