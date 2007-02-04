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
#ifndef BTDOWNLOADER_H
#define BTDOWNLOADER_H

#include <qobject.h>
#include <util/ptrmap.h>
#include "globals.h"

namespace kt
{
	class MonitorInterface;
}


namespace bt
{
	class BitSet;
	class Torrent;
	class ChunkManager;
	class PeerManager;
	class Peer;
	class Chunk;
	class ChunkDownload;
	class PeerDownloader;
	class Piece;
	class Request;
	class ChunkSelector;

	typedef PtrMap<Uint32,ChunkDownload>::iterator CurChunkItr;
	typedef PtrMap<Uint32,ChunkDownload>::const_iterator CurChunkCItr;
	
	#define CURRENT_CHUNK_MAGIC 0xABCDEF00
	
	struct CurrentChunksHeader
	{
		Uint32 magic; // CURRENT_CHUNK_MAGIC
		Uint32 major;
		Uint32 minor;
		Uint32 num_chunks;
	};
	
	/**
	 * @author Joris Guisson
	 * @brief Manages the downloading
	 *
	 * This class manages the downloading of the file. It should
	 * regurarly be updated.
	*/
	class Downloader : public QObject
	{
		Q_OBJECT
		
	public:
		/**
		 * Constructor.
		 * @param tor The Torrent
		 * @param pman The PeerManager
		 * @param cman The ChunkManager
		 */
		Downloader(Torrent & tor,PeerManager & pman,ChunkManager & cman);
		virtual ~Downloader();

		/// Get the number of bytes we have downloaded
		Uint64 bytesDownloaded() const {return downloaded + curr_chunks_downloaded;}

		/// Get the current dowload rate
		Uint32 downloadRate() const;

		/// Get the number of chunks we are dowloading
		Uint32 numActiveDownloads() const {return current_chunks.count();}

		/// See if the download is finished.
		bool isFinished() const;
		
		/**
		 * Clear all downloads. Deletes all active downloads.
		 */
		void clearDownloads();
		
		CurChunkCItr beginDownloads() const {return current_chunks.begin();}
		CurChunkCItr endDownloads() const {return current_chunks.end();}

		/**
		 * See if we are downloading a Chunk
		 * @param chunk ID of Chunk
		 * @return true if we are, false if not
		 */
		bool areWeDownloading(Uint32 chunk) const;

		/**
		 * Save the current downloads.
		 * @param file The file to save to
		 */
		void saveDownloads(const QString & file);

		/**
		 * Load the current downloads.
		 * @param file The file to load from
		 */
		void loadDownloads(const QString & file);

		/**
		 * Get the number of bytes already downloaded in the current_chunks file.
		 * @param file The path of the current_chunks file
		 * @return The bytes already downloading
		 */
		Uint32 getDownloadedBytesOfCurrentChunksFile(const QString & file);
		
		/**
		 * A corrupted chunk has been detected, make sure we redownload it.
		 * @param chunk The chunk
		 */
		void corrupted(Uint32 chunk);
	public slots:
		/**
		 * Update the downloader.
		 */
		void update();
		
		/**
		 * We got a new connection.
		 * @param peer The Peer
		 */
		void onNewPeer(Peer* peer);
		
		/**
		 * A Peer has disconnected.
		 * @param peer The Peer
		 */
		void onPeerKilled(Peer* peer);
		
		/**
		 * Set the TorrentMonitor.
		 * @param tmo 
		 */
		void setMonitor(kt::MonitorInterface* tmo);
		
		static void setMemoryUsage(Uint32 m);
		
		/**
		 * Data has been checked, and these chunks are OK.
		 * @param ok_chunks The ok_chunks
		 */
		void dataChecked(const BitSet & ok_chunks);
		
		/**
		 * Recalculate the number of bytes downloaded.
		 */
		void recalcDownloaded();
		
	private slots:
		void pieceRecieved(const Piece & p);
		bool finished(ChunkDownload* c);
		
		/**
		 * Kill all ChunkDownload's which have been excluded.
		 * @param from First chunk of range
		 * @param to Last chunk of range
		 */
		void onExcluded(Uint32 from,Uint32 to);
		
		/**
		 * Make sure chunk selector is back OK, when chunks are included back again.
		 * @param from First chunk
		 * @param to Last chunk
		 */
		void onIncluded(Uint32 from,Uint32 to);
		
	signals:
		/**
		 * An error occurred while we we're writing or reading from disk.
		 * @param msg Message
		 */
		void ioError(const QString & msg);
		
	private:
		void downloadFrom(PeerDownloader* pd);
		void normalUpdate();
		Uint32 maxMemoryUsage();
		Uint32 numNonIdle();
		bool findDownloadForPD(PeerDownloader* pd,bool warmup);
		ChunkDownload* selectCD(PeerDownloader* pd,Uint32 num);
		ChunkDownload* selectWorst(PeerDownloader* pd);
		
	private:
		Torrent & tor;
		PeerManager & pman;
		ChunkManager & cman;
		Uint64 downloaded;
		Uint64 curr_chunks_downloaded;
		Uint64 unnecessary_data;
		PtrMap<Uint32,ChunkDownload> current_chunks;
		ChunkSelector* chunk_selector;
		
		kt::MonitorInterface* tmon;
		static Uint32 mem_usage;
	};
	


}

#endif
