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

#include <set>
#include <qvaluelist.h>
#include <qobject.h>
#include "globals.h"
#include "request.h"

namespace bt
{
	class Peer;
	class Request;
	class Piece;
	
	typedef std::set<Uint32> AllowedFastSet;
	/**
	 * Request with a timestamp. 
	 */
	struct TimeStampedRequest
	{
		Request req;
		Uint32 time_stamp;
		
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
	class PeerDownloader : public QObject
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
		 * Check for timed out requests.
		 */
		void checkTimeouts();
		
		/// Get the maximum outstanding requests allowed.
		Uint32 getMaximumOutstandingReqs() const;
		
		/// Get the maximum number of chunk downloads
		Uint32 getMaxChunkDownloads() const;
		
		/// Add an allowed fast chunk
		void addAllowedFastChunk(Uint32 chunk);
		
		/**
		 * See if a chunk is an allowed fast chunk
		 * @param chunk The chunk
		 * @return true if the chunk is allowed_fast
		 */
		bool inAllowedFastChunks(Uint32 chunk);
		
		/// Get the number of allowed fast chunks
		Uint32 getNumAllowedFastChunks() const {return allowed_fast.size();}
		
		AllowedFastSet::const_iterator beginAF() const {return allowed_fast.begin();}
		AllowedFastSet::const_iterator endAF() const {return allowed_fast.end();}
		
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
		
		/**
		 * Handles a rejected request.
		 * @param req 
		 */
		void onRejected(const Request & req);
		
		static void setMemoryUsage(Uint32 m);
		
	private slots:
		void piece(const Piece & p);
		void peerDestroyed();
		
	signals:
		/**
		 * Emited when a Piece has been downloaded.
		 * @param p The Piece
		 */
		void downloaded(const Piece & p);
		
		/**
		 * Emitted when a request takes longer then 60 seconds to download.
		 * The sender of the request will have to request it again. This does not apply for
		 * unsent requests. Their timestamps will be updated when they get transmitted.
		 * @param r The request
		 */
		void timedout(const Request & r);
		
		/**
		 * A request was rejected.
		 * @param req The Request
		 */
		void rejected(const Request & req);
		
	private:
		Peer* peer;
		QValueList<TimeStampedRequest> reqs;
		int grabbed;
		AllowedFastSet allowed_fast;
		Uint32 chunk_size;
		static Uint32 max_outstanding_reqs;
	};

}

#endif
