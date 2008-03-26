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

#include "webseed.h"

#include <klocale.h>
#include <kprotocolmanager.h>
#include <util/log.h>
#include <torrent/torrent.h>
#include <diskio/chunkmanager.h>
#include <interfaces/chunkdownloadinterface.h>
#include "httpconnection.h"

namespace bt
{
	class WebSeedChunkDownload : public ChunkDownloadInterface
	{
	public:
		WebSeedChunkDownload(WebSeed* ws,const QString & url,Uint32 index,Uint32 total) 
			: ws(ws),url(url),chunk(index),total_pieces(total),pieces_downloaded(0)
		{}
		 
		virtual ~WebSeedChunkDownload()
		{
		}
		
		void getStats(Stats & s)
		{
			s.current_peer_id = url;
			s.chunk_index = chunk;
			s.num_downloaders = 1;
			s.download_speed = ws->getDownloadRate();
			s.pieces_downloaded = pieces_downloaded;
			s.total_pieces = total_pieces;
		}
		
		WebSeed* ws;
		QString url;
		Uint32 chunk;
		Uint32 total_pieces;
		Uint32 pieces_downloaded;
	};
	
	QString WebSeed::proxy_host;
	Uint16 WebSeed::proxy_port = 8080;
	bool WebSeed::proxy_enabled = false;

	WebSeed::WebSeed(const KUrl & url,bool user,const Torrent & tor,ChunkManager & cman) : WebSeedInterface(url,user),tor(tor),cman(cman)
	{
		first_chunk = last_chunk = tor.getNumChunks() + 1;
		num_failures = 0;
		conn = 0;
		downloaded = 0;
		current = 0;
		status = i18n("Not connected");
	}


	WebSeed::~WebSeed()
	{
		delete conn;
		delete current;
	}
	
	void WebSeed::setProxy(const QString & host,bt::Uint16 port)
	{
		proxy_port = port;
		proxy_host = host;
	}
	
	void WebSeed::setProxyEnabled(bool on)
	{
		proxy_enabled = on;
	}
	
	void WebSeed::reset()
	{
		if (conn)
		{
			delete conn;
			conn = 0;
		}
		
		first_chunk = last_chunk = tor.getNumChunks() + 1;
		num_failures = 0;
		status = i18n("Not connected");
	}

	bool WebSeed::busy() const
	{
		return first_chunk < tor.getNumChunks();
	}
	
	Uint32 WebSeed::getDownloadRate() const
	{
		if (conn)
			return (Uint32)conn->getDownloadRate();
		else
			return 0;
	}
		
	void WebSeed::download(Uint32 first,Uint32 last)
	{
		Out(SYS_CON|LOG_DEBUG) << "WebSeed::download " << first << "-" << last << endl;
		first_chunk = first;
		last_chunk = last;
		cur_chunk = first;
		bytes_of_cur_chunk = 0;
		
		QString path = url.path();
		if (path.endsWith('/'))
			path += tor.getNameSuggestion();
		
		// open connection and connect if needed
		if (!conn)
			conn = new HttpConnection();
		
		if (!conn->connected())
		{
			if (!proxy_enabled)
			{
				QString proxy = KProtocolManager::proxyForUrl(url); // Use KDE settings
				if (proxy.isNull() || proxy == "DIRECT")
					conn->connectTo(url); // direct connection 
				else
				{
					KUrl proxy_url(proxy);
					conn->connectToProxy(proxy_url.host(),proxy_url.port() <= 0 ? 80 : proxy_url.port());
				}
			}
			else 
			{
				if (proxy_host.isNull())
					conn->connectTo(url); // direct connection 
				else
					conn->connectToProxy(proxy_host,proxy_port); // via a proxy
			}
			status = conn->getStatusString();
		}
		
		if (tor.isMultiFile())
		{
			// make the list of ranges to download
			QList<Range> ranges;
			for (Uint32 i = first_chunk;i != last_chunk;i++)
			{
				doChunk(i,ranges);
			}
			
			// add a request for each range
			foreach (const Range & r,ranges)
			{
				const TorrentFile & tf = tor.getFile(r.file);
				conn->get(url.host(),path + '/' + tf.getPath(),r.off,r.len);
			}
		}
		else
		{
			Uint64 len = (last_chunk - first_chunk) * tor.getChunkSize();
			// last chunk can have a different size
			if (last_chunk == tor.getNumChunks() - 1 && tor.getFileLength() % tor.getChunkSize() > 0)
				len += tor.getFileLength() % tor.getChunkSize();
			else
				len += tor.getChunkSize(); 
			
			conn->get(url.host(),path,first_chunk * tor.getChunkSize(),len);
		}
		
		chunkStarted(cur_chunk);
	}
	
	void WebSeed::chunkStarted(Uint32 chunk)
	{
		Uint32 csize = cman.getChunk(chunk)->getSize();
		Uint32 pieces_count = csize / MAX_PIECE_LEN;
		if (csize % MAX_PIECE_LEN > 0)
			pieces_count++;
		current = new WebSeedChunkDownload(this,url.prettyUrl(),chunk,pieces_count);
		chunkDownloadStarted(current);
	}
	
	void WebSeed::chunkStopped()
	{
		if (current)
		{
			chunkDownloadFinished(current);
			delete current;
			current = 0;
		}
	}
		
	Uint32 WebSeed::update()
	{
		if (!conn || !busy())
			return 0;
		
		if (!conn->ok())
		{
			Out(SYS_CON|LOG_DEBUG) << "WebSeed: connection not OK" << endl;
			// shit happened delete connection
			delete conn;
			conn = 0;
			
			chunkStopped();
			
			num_failures++;
			if (num_failures < 3)
			{
				// lets try this again
				download(cur_chunk,last_chunk);
				status = conn->getStatusString();
			}
			else
				status = i18n("Error, failed to connect");
			return 0;
		}
		else if (conn->closed())
		{
			Out(SYS_CON|LOG_DEBUG) << "WebSeed: connection closed" << endl;
			delete conn;
			conn = 0;
			
			status = i18n("Connection closed");
			chunkStopped();
			// lets try this again
			download(cur_chunk,last_chunk);
			status = conn->getStatusString();
		}
		else
		{
			QByteArray tmp;
			while (conn->getData(tmp) && cur_chunk <= last_chunk)
			{
				//Out(SYS_CON|LOG_DEBUG) << "WebSeed: handleData " << tmp.size() << endl;
				handleData(tmp);
				tmp.clear();
			}
			
			if (cur_chunk > last_chunk)
			{
				// if the current chunk moves past the last chunk, we are done
				first_chunk = last_chunk = tor.getNumChunks() + 1;
				num_failures = 0;
				finished();
			}
			status = conn->getStatusString();
		}
		
		Uint32 ret = downloaded;
		downloaded = 0;
		total_downloaded += ret;
		return ret;
	}
	
	void WebSeed::handleData(const QByteArray & tmp)
	{
		Uint32 off = 0;
		while (off < (Uint32)tmp.size() && cur_chunk <= last_chunk)
		{
			Chunk* c = cman.getChunk(cur_chunk);
			Uint32 bl = c->getSize() - bytes_of_cur_chunk;
			if (bl > tmp.size() - off)
				bl = tmp.size() - off;
					
			if (c->getStatus() == Chunk::BUFFERED || c->getStatus() == Chunk::MMAPPED)
			{
				// only write when we have the chunk in memory
				// if we already have the chunk we will then just ignore the data
				memcpy(c->getData() + bytes_of_cur_chunk,tmp.data() + off,bl);
				downloaded += bl;
			}
			off += bl;
			bytes_of_cur_chunk += bl;
			current->pieces_downloaded = bytes_of_cur_chunk / MAX_PIECE_LEN;
			if (bytes_of_cur_chunk == c->getSize())
			{
				// we have one ready
				bytes_of_cur_chunk = 0;
				cur_chunk++;
				if (c->getStatus() == Chunk::BUFFERED || c->getStatus() == Chunk::MMAPPED)
					chunkReady(c);
				
				chunkStopped();
				if (cur_chunk <= last_chunk)
					chunkStarted(cur_chunk);
			}
		}
	}

	void WebSeed::doChunk(Uint32 chunk,QList<Range> & ranges)
	{
		QList<Uint32> tflist;
		tor.calcChunkPos(chunk,tflist);
		Chunk* c = cman.getChunk(chunk);
		
		Uint64 passed = 0; // number of bytes of the chunk which we have passed
		for (int i = 0;i < tflist.count();i++)
		{
			const TorrentFile & tf = tor.getFile(tflist[i]);
			Range r = {tflist[i],0,0};
			if (i == 0)
				r.off = tf.fileOffset(chunk,tor.getChunkSize());
		
			if (tflist.count() == 1)
				r.len = c->getSize();
			else if (i == 0)
				r.len = tf.getLastChunkSize();
			else if (i == tflist.count() - 1)
				r.len = c->getSize() - passed;
			else
				r.len = tf.getSize();
			
			// add the range
			if (ranges.count() == 0)
				ranges.append(r);
			else if (ranges.back().file != r.file)
				ranges.append(r);
			else
			{
				// the last range and this one are in the same file
				// so expand it
				Range & l = ranges.back();
				l.len += r.len;
			}
			
			passed += r.len;
		}
	}

	void WebSeed::onExcluded(Uint32 from,Uint32 to)
	{
		if (from <= first_chunk <= to && from <= last_chunk <= to)
			reset();
	}
}
#include "webseed.moc"
