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
#ifndef BTPEERDOWNLOADER_H
#define BTPEERDOWNLOADER_H

#include <qvaluelist.h>
#include <qobject.h>
#include "globals.h"

namespace bt
{
	class Peer;
	class Request;
	class Piece;
	

	/**
	 * @author Joris Guisson
	 * @brief Class which downloads pieces from a Peer
	 *
	 * This class downloads Piece's from a Peer.
	*/
	class PeerDownloader : public QObject
	{
		Q_OBJECT
	public:
		/**
		 * Constructor, set the Peer
		 * @param peer The Peer
		 */
		PeerDownloader(Peer* peer);
		virtual ~PeerDownloader();

		/// Get the number of active requests
		Uint32 getNumRequests() const;

		/// Is the Peer choked.
		bool isChoked() const;

		/// Is NULL (is the Peer set)
		bool isNull() const {return peer == 0;}

		/**
		 * See if the Peer has a Chunk
		 * @param idx The Chunk's index
		 */
		bool hasChunk(Uint32 idx) const;
		
		/**
		 * Grab the Peer, indicates how many ChunkDownload's
		 * are using this PeerDownloader.
		 * @return The number of times this PeerDownloader was grabbed
		 */
		int grab();
		
		/**
		 * When a ChunkDownload is ready with this PeerDownloader,
		 * it will release it, so that others can use it.
		 */
		void release();

		/// Get the number of times this PeerDownloader was grabbed.
		int getNumGrabbed() const {return grabbed;}

		/// Get the Peer
		const Peer* getPeer() const {return peer;}

		/// Get the current download rate
		Uint32 getDownloadRate() const;
		
		/**
		 * Try to download unsent requests.
		 */
		void downloadUnsent();

		/**
		 * Set the interval between two requests.
		 * Used by the DownloadCap to tweak the download
		 * speed.
		 * @param rti Interval in milliseconds
		 */
		void setRequestInterval(Uint32 rti);
		
	public slots:
		/**
		 * Send a Request. Note that the DownloadCap
		 * may not allow this. (In which case it will
		 * be stored temporarely in the unsent_reqs list)
		 * @param req The Request
		 */
		void download(const Request & req);

		/**
		 * Cancel a Request.
		 * @param req The Request
		 */
		void cancel(const Request & req);

		/**
		 * Cancel all Requests
		 */
		void cancelAll();
		
	private slots:
		void piece(const Piece & p);
		void peerDestroyed();
		
	signals:
		/**
		 * Emited when a Piece has been downloaded.
		 * @param p The Piece
		 */
		void downloaded(const Piece & p);
		
	private:
		Peer* peer;
		Uint32 last_req_time,req_time_interval;
		QValueList<Request> reqs;
		QValueList<Request> unsent_reqs;
		int grabbed;
	};

}

#endif
