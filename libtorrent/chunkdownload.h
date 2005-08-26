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
#ifndef BTCHUNKDOWNLOAD_H
#define BTCHUNKDOWNLOAD_H

#include <set>
#include <qobject.h>
#include <qptrlist.h>
#include <libutil/timer.h>
#include <libutil/ptrmap.h>
#include "globals.h"
#include "peerid.h"

namespace bt
{
	
	class File;
	class Chunk;
	class Piece;
	class Peer;
	class Request;
	class PeerDownloader;
	class DownloadStatus;
	
	
	/**
	 * @author Joris Guisson
	 * @brief Handles the download off one Chunk off a Peer
	 * 
	 * This class handles the download of one Chunk.
	*/
	class ChunkDownload : public QObject
	{
		Q_OBJECT
	public:
		/**
		 * Constructor, set the chunk and the PeerManager.
		 * @param chunk The Chunk
		 */
		ChunkDownload(Chunk* chunk);
		
		virtual ~ChunkDownload();

		Chunk* getChunk() {return chunk;}
	
		Uint32 totalPieces() const {return num;}
		Uint32 piecesDownloaded() const {return num_downloaded;}

		/// Get the number of bytes downloaded.
		Uint32 bytesDownloaded() const;
		
		Uint32 getChunkIndex() const;
		const Peer* getCurrentPeer() const;
		QString getCurrentPeerID() const;
		Uint32 getDownloadSpeed() const;
		
		/**
		 * A Piece has arived.
		 * @param p The Piece
		 * @return true If Chunk is complete
		 */
		bool piece(const Piece & p);	
		
		/**
		 * Assign the downloader to download from.
		 * @param pd The downloader
		 * @param endgame Wether or not we are in endgame mode
		 */
		void assignPeer(PeerDownloader* pd,bool endgame);
		
		Uint32 getNumDownloaders() const {return pdown.count();}

		/**
		 * A Peer has been killed. We need to remove it's
		 * PeerDownloader.
		 * @param pd The PeerDownloader
		 */
		void peerKilled(PeerDownloader* pd);

		/**
		 * Save to a File
		 * @param file The File
		 */
		void save(File & file);
		
		/**
		 * Load from a File
		 * @param file The File
		 */
		void load(File & file);
	private slots:
		void sendRequests(PeerDownloader* pd);
		void sendCancels(PeerDownloader* pd);
		void endgameCancel(const Piece & p);
		
	private:
		
		bool* pieces;
		Chunk* chunk;
		Uint8* buf;
		Uint32 num;
		Uint32 num_downloaded;
		Uint32 last_size;
		Timer timer;
		QPtrList<PeerDownloader> pdown;
		PtrMap<Uint32,DownloadStatus> dstatus;

		friend File & operator << (File & out,const ChunkDownload & cd);
		friend File & operator >> (File & in,ChunkDownload & cd);
	};
}

#endif
