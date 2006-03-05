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
#ifndef BTCHUNKSELECTOR_H
#define BTCHUNKSELECTOR_H

#include <list>
#include <util/timer.h>

namespace bt
{
	class PeerDownloader;
	class ChunkManager;
	class Downloader;
	class PeerManager;
	/**
	 * @author Joris Guisson
	 *
	 * Selects which Chunks to download. 
	*/
	class ChunkSelector
	{
		ChunkManager & cman;
		Downloader & downer;
		PeerManager & pman;
		std::list<Uint32> chunks;
		Timer sort_timer;
	public:
		ChunkSelector(ChunkManager & cman,Downloader & downer,PeerManager &pman);
		virtual ~ChunkSelector();

		/**
		 * Select which chunk to download for a PeerDownloader.
		 * @param pd The PeerDownloader
		 * @param chunk Index of chunk gets stored here
		 * @return true upon succes, false otherwise
		 */
		bool select(PeerDownloader* pd,Uint32 & chunk);
	};

}

#endif

