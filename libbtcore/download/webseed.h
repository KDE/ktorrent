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
#include <btcore_export.h>
#include <util/constants.h>
#include <interfaces/webseedinterface.h>
#include <interfaces/chunkdownloadinterface.h>
#include <diskio/piecedata.h>


namespace bt
{
	class Torrent;
	class HttpConnection;
	class ChunkManager;
	class Chunk;
	class WebSeedChunkDownload;
	

	/**
		@author Joris Guisson
		Class which handles downloading from a webseed
	*/
	class BTCORE_EXPORT WebSeed : public QObject,public WebSeedInterface
	{
		Q_OBJECT
	public:
		WebSeed(const KUrl & url,bool user,const Torrent & tor,ChunkManager & cman);
		virtual ~WebSeed();
		
		/// Is this webseed busy ?
		bool busy() const;
		
		/// Check if a chunk lies in the current range we are downloading
		bool inCurrentRange(Uint32 chunk) const {return chunk >= first_chunk && chunk <= last_chunk;}
		
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
		 * A chunk has been downloaded.
		 * @param chunk The chunk
		 */
		void chunkDownloaded(Uint32 chunk);
		
		/**
		 * Reset the webseed (kills the connection)
		 */
		void reset();
		
		/**
		* Cancel the current download and kill the connection
		*/
		void cancel();
		
		/// Get the current download rate
		Uint32 getDownloadRate() const;
		
			
		/**
		 * Set the group ID's of the http connection (for speed limits)
		 * @param up Upload group id
		 * @param down Download group id
		 */
		void setGroupIDs(Uint32 up,Uint32 down);
		
		
		/**
		 * Set the proxy to use for all WebSeeds
		 * @param host Hostname or IP address of the proxy
		 * @param port Port number of the proxy
		 */
		static void setProxy(const QString & host,bt::Uint16 port);
		
		/**
		 * Whether or not to enable or disable the use of a proxy.
		 * When the proxy is disabled, we will use the KDE proxy settings.
		 * @param on On or not
		 */
		static void setProxyEnabled(bool on);
		
		/// Get the current webseed download
		WebSeedChunkDownload* currentChunkDownload() {return current;}
		
		virtual void setEnabled(bool on);
		
		/// Get the number of failed attempts
		Uint32 failedAttempts() const {return num_failures;}
		
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
		 * @param chunk The chunk which is being started
		 */
		void chunkDownloadStarted(WebSeedChunkDownload* cd,Uint32 chunk);
		
		/**
		 * A ChunkDownload was finished
		 * @param cd The ChunkDownloadInterface
		 * @param chunk The chunk which is being stopped
		 */
		void chunkDownloadFinished(WebSeedChunkDownload* cd,Uint32 chunk);
		
	private slots:
		void redirected(const KUrl & to_url);
		
	private:
		struct Range
		{
			Uint32 file;
			Uint64 off;
			Uint64 len;
		};
		
		void fillRangeList(Uint32 chunk);
		void handleData(const QByteArray & data);
		void chunkStarted(Uint32 chunk);
		void chunkStopped();
		void connectToServer();
		void continueCurChunk();
		void readData();
		
	private:
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
		Uint32 up_gid,down_gid;
		QList<Range> range_queue;
		KUrl redirected_url;
		PieceDataPtr cur_piece;
		
		static QString proxy_host;
		static Uint16 proxy_port;
		static bool proxy_enabled;
	};
	
	class WebSeedChunkDownload : public ChunkDownloadInterface
	{
	public:
		WebSeedChunkDownload(WebSeed* ws,const QString & url,Uint32 index,Uint32 total);
		virtual ~WebSeedChunkDownload();
	
		virtual void getStats(Stats & s);
	
		WebSeed* ws;
		QString url;
		Uint32 chunk;
		Uint32 total_pieces;
		Uint32 pieces_downloaded;
	};

}

#endif
