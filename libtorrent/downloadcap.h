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

#include <libutil/timer.h>
#include "globals.h"

namespace bt
{

	/**
	 * @author Joris Guisson
	*/
	class DownloadCap
	{
		static Uint32 max_bytes_per_sec;
		static Uint32 current_speed;
		static Uint32 outstanding_bytes;
		static Timer timer;
	public:

		/**
		 * Set the speed cap in bytes per second. 0 indicates
		 * no limit.
		 * @param max Maximum number of bytes per second.
		*/
		static void setMaxSpeed(Uint32 max);

		/**
		 * Set the current download speed. 
		 * @param s The speed (in bytes per sec)
		 */
		static void setCurrentSpeed(Uint32 s);

		/// Get the maximum speed (0 == no limit)
		static Uint32 getMaxSpeed() {return max_bytes_per_sec;}
		
		/**
		 * Request permission to request a piece.
		 * @param The number of bytes we want to download
		 * @return true if we can, false if not
		 */
		static bool allow(Uint32 bytes);

		/**
		 * We recieved a number of bytes. 
		 * @param bytes 
		 */
		static void recieved(Uint32 bytes);
	};

}

#endif
