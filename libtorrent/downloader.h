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
#ifndef BTDOWNLOADER_H
#define BTDOWNLOADER_H

#include <qobject.h>
#include "globals.h"
#include "ptrmap.h"


namespace bt
{
	class Torrent;
	class ChunkManager;
	class PeerManager;
	class Peer;
	class Chunk;
	class ChunkDownload;
	class PeerDownloader;
	class Piece;
	class Request;
	class TorrentMonitor;

	typedef PtrMap<Uint32,ChunkDownload>::iterator CurChunkItr;
	typedef PtrMap<Uint32,ChunkDownload>::const_iterator CurChunkCItr;
	
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
		Uint32 bytesDownloaded() const {return downloaded;}

		/// Get the current dowload rate
		Uint32 downloadRate() const;

		/// Get the number of chunks we are dowloading
		Uint32 numActiveDownloads() const {return current_chunks.count();}
		
		/**
		 * Clear all downloads. Deletes all active downloads.
		 */
		void clearDownloads();
		
		CurChunkCItr beginDownloads() const {return current_chunks.begin();}
		CurChunkCItr endDownloads() const {return current_chunks.end();}

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
		void setMonitor(TorrentMonitor* tmo);
		
	private slots:
		void pieceRecieved(const Piece & p);
		void finished(ChunkDownload* c);
		
	private:
		void downloadFrom(PeerDownloader* pd);
		void normalUpdate();
		void endgameUpdate();
		
	private:
		Torrent & tor;
		PeerManager & pman;
		ChunkManager & cman;
		Uint32 downloaded;
		PtrMap<Uint32,ChunkDownload> current_chunks;
		PtrMap<Peer*,PeerDownloader> pdowners;
		bool endgame_mode;
		
		TorrentMonitor* tmon;
	};
	


};

#endif
