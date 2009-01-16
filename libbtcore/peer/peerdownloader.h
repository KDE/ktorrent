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
#ifndef BTPEERDOWNLOADER_H
#define BTPEERDOWNLOADER_H

#include <qlist.h>
#include <qobject.h>
#include <interfaces/piecedownloader.h>
#include <download/request.h>

namespace bt
{
	class Peer;
	class Request;
	class Piece;
	
	/**
	 * Request with a timestamp. 
	 */
	struct TimeStampedRequest
	{
		Request req;
		TimeStamp time_stamp;
		
		TimeStampedRequest();
		
		/**
		 * Constructor, set the request and calculate the timestamp.
		 * @param r The Request
		 */
		TimeStampedRequest(const Request & r);
		
		/**
		 * Copy constructor, copy the request and the timestamp
		 * @param r The Request
		 */
		TimeStampedRequest(const TimeStampedRequest & t);
		
		/// Destructor
		~TimeStampedRequest();

		/**
		 * Equality operator, compares requests only.
		 * @param r The Request
		 * @return true if equal
		 */
		bool operator == (const Request & r);
		
		/**
		 * Equality operator, compares requests only.
		 * @param r The Request
		 * @return true if equal
		 */
		bool operator == (const TimeStampedRequest & r);
		
		/**
		 * Assignment operator.
		 * @param r The Request to copy
		 * @return *this
		 */
		TimeStampedRequest & operator = (const Request & r);
		
		/**
		 * Assignment operator.
		 * @param r The TimeStampedRequest to copy
		 * @return *this
		 */
		TimeStampedRequest & operator = (const TimeStampedRequest & r);
	};
	

	/**
	 * @author Joris Guisson
	 * @brief Class which downloads pieces from a Peer
	 *
	 * This class downloads Piece's from a Peer.
	*/
	class PeerDownloader : public PieceDownloader
	{
		Q_OBJECT	
	public:
		/**
		 * Constructor, set the Peer
		 * @param peer The Peer
		 * @param chunk_size Size of a chunk in bytes
		 */
		PeerDownloader(Peer* peer,Uint32 chunk_size);
		virtual ~PeerDownloader();

		/// See if we can add a request to the wait_queue
		virtual bool canAddRequest() const;
		virtual bool canDownloadChunk() const;
		
		/// Get the number of active requests
		Uint32 getNumRequests() const;

		/// Is the Peer choked.
		virtual bool isChoked() const;

		/// Is NULL (is the Peer set)
		bool isNull() const {return peer == 0;}

		/**
		 * See if the Peer has a Chunk
		 * @param idx The Chunk's index
		 */
		virtual bool hasChunk(Uint32 idx) const;
		
		/// Get the Peer
		const Peer* getPeer() const {return peer;}

		/**
		 * Check for timed out requests.
		 */
		void checkTimeouts();
		
		/// Get the maximum number of chunk downloads
		Uint32 getMaxChunkDownloads() const;
				
		/**
		 * The peer has been choked, all pending requests are rejected.
		 * (except for allowed fast ones)
		 */
		void choked();
		
		virtual QString getName() const;
		virtual Uint32 getDownloadRate() const;
		
		/**
		 * Called when a piece has arrived.
		 * @param p The Piece
		 */
		void piece(const Piece & p);
		
	public slots:
		/**
		 * Send a Request. Note that the DownloadCap
		 * may not allow this. (In which case it will
		 * be stored temporarely in the unsent_reqs list)
		 * @param req The Request
		 */
		virtual void download(const Request & req);

		/**
		 * Cancel a Request.
		 * @param req The Request
		 */
		virtual void cancel(const Request & req);

		/**
		 * Cancel all Requests
		 */
		virtual void cancelAll();
		
		/**
		 * Handles a rejected request.
		 * @param req 
		 */
		void onRejected(const Request & req);
		
		/**
		 * Send requests and manage wait queue
		 */
		void update();
		
	private slots:
		void peerDestroyed();
		
		
	private:
		Peer* peer;
		QList<TimeStampedRequest> reqs;
		QList<Request> wait_queue;
		Uint32 max_wait_queue_size;
		Uint32 chunk_size;
	};

}

#endif
