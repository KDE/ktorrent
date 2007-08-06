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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef KTCHUNKDOWNLOADINTERFACE_H
#define KTCHUNKDOWNLOADINTERFACE_H

#include <qstring.h>
#include <util/constants.h>

namespace kt
{

	/**
	 * @author Joris Guisson
	 * @brief Interface for a ChunkDownload
	 *
	 * This class provides the interface for a ChunkDownload object.
	*/
	class ChunkDownloadInterface
	{
	public:
		ChunkDownloadInterface();
		virtual ~ChunkDownloadInterface();

		struct Stats
		{
			/// The PeerID of the current downloader
			QString current_peer_id;
			/// The current download speed
			bt::Uint32 download_speed;
			/// The index of the chunk
			bt::Uint32 chunk_index;
			/// The number of pieces of the chunk which have been downloaded
			bt::Uint32 pieces_downloaded;
			/// The total number of pieces of the chunk
			bt::Uint32 total_pieces;
			/// The number of assigned downloaders
			bt::Uint32 num_downloaders;
		};

		virtual void getStats(Stats & s) = 0;
	};

}

#endif
