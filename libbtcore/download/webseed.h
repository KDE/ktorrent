/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef BTWEBSEED_H
#define BTWEBSEED_H

#include <QObject>
#include <kurl.h>
#include <util/constants.h>

namespace bt
{
	class Torrent;
	class HttpConnection;
	class ChunkManager;
	class Chunk;
	class ChunkDownloadInterface;
	class WebSeedChunkDownload;

	/**
		@author Joris Guisson
		Class which handles downloading from a webseed
	*/
	class WebSeed : public QObject
	{
		Q_OBJECT
	public:
		WebSeed(const KUrl & url,const Torrent & tor,ChunkManager & cman);
		virtual ~WebSeed();
		
		/// Is this webseed busy ?
		bool busy() const;
		
		/**
		 * Download a range of chunks
		 * @param first The first chunk
		 * @param last The last chunk
		 */
		void download(Uint32 first,Uint32 last);
		
		/**
		 * A range has been excluded, if we are fully
		 * downloading in this range, reset.
		 * @param from Start of range
		 * @param to End of range
		 */
		void onExcluded(Uint32 from,Uint32 to);
		
		/**
		 * Check if the connection has received some data and handle it.
		 * @return The number of bytes downloaded
		 */
		Uint32 update();
		
		/**
		 * Reset the webseed (kills the connection)
		 */
		void reset();
		
		/// Get the current download rate
		Uint32 getDownloadRate() const;
		
	signals:
		/**
		 * Emitted when a chunk is downloaded
		 * @param c The chunk
		 */
		void chunkReady(Chunk* c);
		
		/**
		 * Emitted when a range has been fully downloaded
		 */
		void finished();
		
		/**
		 * A ChunkDownload was started
		 * @param cd The ChunkDownloadInterface
		 */
		void chunkDownloadStarted(ChunkDownloadInterface* cd);
		
		/**
		 * A ChunkDownload was finished
		 * @param cd The ChunkDownloadInterface
		 */
		void chunkDownloadFinished(ChunkDownloadInterface* cd);
		
	private:
		struct Range
		{
			Uint32 file;
			Uint64 off;
			Uint64 len;
		};
		
		void doChunk(Uint32 chunk,QList<Range> & ranges);
		void handleData(const QByteArray & data);
		void chunkStarted(Uint32 chunk);
		void chunkStopped();
		
	private:
		KUrl url;
		const Torrent & tor;
		ChunkManager & cman;
		HttpConnection* conn;
		QList<QByteArray> chunks;
		Uint32 first_chunk;
		Uint32 last_chunk;
		Uint32 cur_chunk;
		Uint32 bytes_of_cur_chunk;
		Uint32 num_failures;
		Uint32 downloaded;
		WebSeedChunkDownload* current;
	};

}

#endif
