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
#ifndef BTMONITORINTERFACE_H
#define BTMONITORINTERFACE_H

#include <btcore_export.h>

namespace bt
{
	class ChunkDownloadInterface;
	class PeerInterface;
	class TorrentFileInterface;

	/**
	 * @author Joris Guisson
	 * @brief Interface for classes who want to monitor a TorrentInterface
	 *
	 * Classes who want to keep track of all peers currently connected for a given
	 * torrent and all chunks who are currently downloading can implement this interface.
	 */
	class BTCORE_EXPORT MonitorInterface
	{
	public:
		MonitorInterface();
		virtual ~MonitorInterface();

		/**
		 * A peer has been added.
		 * @param peer The peer
		 */
		virtual void peerAdded(PeerInterface* peer) = 0;

		/**
		 * A peer has been removed.
		 * @param peer The peer
		 */
		virtual void peerRemoved(PeerInterface* peer) = 0;
		
		/**
		 * The download of a chunk has been started.
		 * @param cd The ChunkDownload
		 */
		virtual void downloadStarted(ChunkDownloadInterface* cd) = 0;

		/**
		 * The download of a chunk has been stopped.
		 * @param cd The ChunkDownload
		 */
		virtual void downloadRemoved(ChunkDownloadInterface* cd) = 0;
		
		/**
		 * The download has been stopped.
		 */
		virtual void stopped() = 0;

		/**
		 * The download has been deleted.
		 */
		virtual void destroyed() = 0;
		
		/**
		 * The download percentage of a file has changed. 
		 * @param file The file
		 * @param percentage The percentage
		 */
		virtual void filePercentageChanged(TorrentFileInterface* file,float percentage) = 0;
		
		/**
		 * Preview status of a file has changed.
		 * @param file The file
		 * @param preview Wether or not it is available
		 */
		virtual void filePreviewChanged(TorrentFileInterface* file,bool preview) = 0;
	};

}

#endif
