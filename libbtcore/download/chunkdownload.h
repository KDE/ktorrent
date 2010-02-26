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
#ifndef BTCHUNKDOWNLOAD_H
#define BTCHUNKDOWNLOAD_H

#include <QSet>
#include <QObject>
#include <QList>
#include <util/timer.h>
#include <util/ptrmap.h>
#include <util/sha1hashgen.h>
#include <interfaces/chunkdownloadinterface.h>
#include <util/bitset.h>


namespace bt
{
	
	class File;
	class Chunk;
	class Piece;
	class Peer;
	class Request;
	class PieceDownloader;
	class PieceData;
	class PieceDataPtr;
	
	struct ChunkDownloadHeader
	{
		Uint32 index;
		Uint32 num_bits;
		Uint32 buffered;
	};
	
	struct PieceHeader
	{
		Uint32 piece;
		Uint32 size; 
		Uint32 mapped;
	};

	class DownloadStatus
	{
	public:
		DownloadStatus();
		~DownloadStatus();

		void add(Uint32 p);
		void remove(Uint32 p);
		bool contains(Uint32 p);
		void clear();
		
		void timeout() {timeouts++;}
		Uint32 numTimeouts() const {return timeouts;}
		
		typedef QSet<Uint32>::iterator iterator;
		iterator begin() {return status.begin();}
		iterator end() {return status.end();}
		
	private:
		Uint32 timeouts;
		QSet<Uint32> status;
	};
	
	
	
	/**
	 * @author Joris Guisson
	 * @brief Handles the download off one Chunk off a Peer
	 * 
	 * This class handles the download of one Chunk.
	*/
	class ChunkDownload : public QObject,public ChunkDownloadInterface 
	{
		Q_OBJECT
	public:
		/**
		 * Constructor, set the chunk and the PeerManager.
		 * @param chunk The Chunk
		 */
		ChunkDownload(Chunk* chunk);
		
		virtual ~ChunkDownload();

		/// Get the chunk
		Chunk* getChunk() {return chunk;}
	
		/// Get the total number of pieces
		Uint32 getTotalPieces() const {return num;}
		
		/// Get the number of pieces downloaded
		Uint32 getPiecesDownloaded() const {return num_downloaded;}

		/// Get the number of bytes downloaded.
		Uint32 bytesDownloaded() const;
		
		/// Get the index of the chunk
		Uint32 getChunkIndex() const;
		
		/// Get the PeerID of the current peer
		QString getPieceDownloaderName() const;
		
		/// Get the download speed
		Uint32 getDownloadSpeed() const;

		/// Get download stats
		void getStats(Stats & s);
		
		/// See if a chunkdownload is idle (i.e. has no downloaders)
		bool isIdle() const {return pdown.count() == 0;}
		
		/**
		 * A Piece has arived.
		 * @param p The Piece
		 * @param ok Whether or not the piece was needed
		 * @return true If Chunk is complete
		 */
		bool piece(const Piece & p,bool & ok);	
		
		/**
		 * Assign the downloader to download from.
		 * @param pd The downloader
		 * @return true if the peer was asigned, false if not
		 */
		bool assign(PieceDownloader* pd);

		/**
		 * A PieceDownloader has been killed. We need to remove it.
		 * @param pd The PieceDownloader
		 */
		void killed(PieceDownloader* pd);

		/**
		 * Save to a File
		 * @param file The File
		 */
		void save(File & file);
		
		/**
		 * Load from a File
		 * @param file The File
		 * @param hdr Header for the chunk
		 * @param update_hash Wether or not to update the hash
		 */
		bool load(File & file,ChunkDownloadHeader & hdr,bool update_hash = true);

		/**
		 * Cancel all requests.
		 */
		void cancelAll();

		/**
		 * When a Chunk is downloaded, this function checks if all
		 * pieces are delivered by the same peer and if so returns it.
		 * @return The PieceDownloader or 0 if there is no only peer
		 */
		PieceDownloader* getOnlyDownloader();
		
		/// See if a PieceDownloader is assigned to this chunk
		bool containsPeer(PieceDownloader *pd) {return pdown.contains(pd);} 
		
		/// See if the download is choked (i.e. all downloaders are choked)
		bool isChoked() const;
		
		/// Release all PD's and clear the requested chunks
		void releaseAllPDs();
		
		/// Send requests to peers
		void update();
		
		/// See if this CD hasn't been active in the last update
		bool needsToBeUpdated() const {return timer.getElapsedSinceUpdate() > 60 * 1000;}
		
		/// Get the SHA1 hash of the downloaded chunk
		SHA1Hash getHash() const {return hash_gen.get();}
		
		/// Get the number of downloaders
		Uint32 getNumDownloaders() const {return pdown.count();}

	private slots:
		void onTimeout(const bt::Request & r);
		void onRejected(const bt::Request & r);
		
	private:
		void notDownloaded(const Request & r,bool reject);
		void updateHash();
		void sendRequests();
		bool sendRequest(PieceDownloader* pd);
		void sendCancels(PieceDownloader* pd);
		void endgameCancel(const Piece & p);
		Uint32 bestPiece(PieceDownloader* pd);
		
	private:		
		BitSet pieces;
		Chunk* chunk;
		Uint32 num;
		Uint32 num_downloaded;
		Uint32 last_size;
		Timer timer;
		QList<PieceDownloader*> pdown;
		PtrMap<PieceDownloader*,DownloadStatus> dstatus;
		QSet<PieceDownloader*> piece_providers;
		PieceDataPtr* piece_data;
		SHA1HashGen hash_gen;
		Uint32 num_pieces_in_hash;

		friend File & operator << (File & out,const ChunkDownload & cd);
		friend File & operator >> (File & in,ChunkDownload & cd);
	};
}

#endif
