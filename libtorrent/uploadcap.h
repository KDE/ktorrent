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
#ifndef BTUPLOADCAP_H
#define BTUPLOADCAP_H

#include "timer.h"
#include "globals.h"

namespace bt
{

	/**
	 * @author Joris Guisson
	 * @brief Keeps the upload rate under control
	 * 
	 * Before a PeerUploader can't send a piece, it must first ask
	 * permission to a UploadCap object. This object will make sure
	 * that the upload rate remains under a specified threshold. When the
	 * threshold is set to 0, no upload capping will be done.
	*/
	class UploadCap
	{
		static Uint32 max_bytes_per_sec;
		static Timer timer;
	public:

		/**
		 * Set the speed cap in bytes per second. 0 indicates
		 * no limit.
		 * @param max Maximum number of bytes per second.
		 */
		static void setSpeed(Uint32 max);
		
		static Uint32 getSpeed() {return max_bytes_per_sec;}
		
		/**
		 * Request permission to send a piece @a bytes large.
		 * @param bytes The size of the piece
		 * @return The number of bytes we can upload
		 */
		static Uint32 allow(Uint32 bytes);
	};

}

#endif
