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

#include <QTimer>
#include <klocale.h>
#include <kprotocolmanager.h>
#include <util/log.h>
#include <torrent/torrent.h>
#include <diskio/chunkmanager.h>
#include <diskio/piecedata.h>
#include <net/socketmonitor.h>
#include "httpconnection.h"

namespace bt
{
	
			
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
		up_gid = down_gid = 0;
		cur_chunk = -1;
	}


	WebSeed::~WebSeed()
	{
		delete conn;
		delete current;
	}
	
	void WebSeed::setGroupIDs(Uint32 up,Uint32 down)
	{
		up_gid = up;
		down_gid = down;
		if (conn)
			conn->setGroupIDs(up,down);
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
		if (current)
			chunkStopped();
		
		if (conn)
		{
			conn->deleteLater();
			conn = 0;
		}
		
		first_chunk = last_chunk = tor.getNumChunks() + 1;
		num_failures = 0;
		status = i18n("Not connected");
	}
	
	void WebSeed::cancel() 
	{
		reset();
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
	
	void WebSeed::connectToServer()
	{
		KUrl dst = url;
		if (redirected_url.isValid())
			dst = redirected_url;
		
		if (!proxy_enabled)
		{
			QString proxy = KProtocolManager::proxyForUrl(dst); // Use KDE settings
			if (proxy.isNull() || proxy == "DIRECT")
				conn->connectTo(dst); // direct connection 
			else
			{
				KUrl proxy_url(proxy);
				conn->connectToProxy(proxy_url.host(),proxy_url.port() <= 0 ? 80 : proxy_url.port());
			}
		}
		else 
		{
			if (proxy_host.isNull())
				conn->connectTo(dst); // direct connection 
			else
				conn->connectToProxy(proxy_host,proxy_port); // via a proxy
		}
		status = conn->getStatusString();
	}
		
	void WebSeed::download(Uint32 first,Uint32 last)
	{
		if (!enabled)
			return;
		
		//Out(SYS_CON|LOG_DEBUG) << "WebSeed: downloading " << first << "-" << last << " from " << url.prettyUrl() << endl;
		// open connection and connect if needed
		if (!conn)
		{
			conn = new HttpConnection();
			conn->setGroupIDs(up_gid,down_gid);
		}
		
		if (!conn->connected())
		{
			connectToServer();
		}
		
		if (first == cur_chunk && last == last_chunk && bytes_of_cur_chunk > 0)
		{
			// we already have some of the data, so reuse it
			continueCurChunk();
			return;
		}
		
		cur_piece = PieceDataPtr(0);
		first_chunk = first;
		last_chunk = last;
		cur_chunk = first;
		bytes_of_cur_chunk = 0;
		
		QString path = url.path();
		if (path.endsWith('/') && !isUserCreated())
			path += tor.getNameSuggestion();
		
		if (tor.isMultiFile())
		{
			range_queue.clear();
			// make the list of ranges to download
			for (Uint32 i = first_chunk;i <= last_chunk;i++)
			{
				fillRangeList(i);
			}
			
			if (range_queue.count() > 0)
			{
				// send the first request
				Range r = range_queue[0];
				range_queue.pop_front();
				const TorrentFile & tf = tor.getFile(r.file);
				QString host = redirected_url.isValid() ? redirected_url.host() : url.host();
				conn->get(host,path + '/' + tf.getPath(),r.off,r.len);
			}
		}
		else
		{
			Uint64 len = (last_chunk - first_chunk) * tor.getChunkSize();
			// last chunk can have a different size
			if (last_chunk == tor.getNumChunks() - 1)
				len += tor.getLastChunkSize();
			else
				len += tor.getChunkSize(); 
			
			QString host = redirected_url.isValid() ? redirected_url.host() : url.host();
			conn->get(host,path,first_chunk * tor.getChunkSize(),len);
		}
	}
	
	void WebSeed::continueCurChunk()
	{
		QString path = url.path();
		if (path.endsWith('/') && !isUserCreated())
			path += tor.getNameSuggestion();
		
		//Out(SYS_GEN|LOG_DEBUG) << "WebSeed: continuing current chunk " << cur_chunk << " " << bytes_of_cur_chunk << endl;
		first_chunk = cur_chunk;
		if (tor.isMultiFile())
		{
			range_queue.clear();
			// make the list of ranges to download
			for (Uint32 i = first_chunk;i <= last_chunk;i++)
			{
				fillRangeList(i);
			}
			
			bt::Uint32 length = 0;
			while (range_queue.count() > 0)
			{
				// send the first request, but skip the data we already have
				Range r = range_queue[0];
				range_queue.pop_front();
				if (length >= bytes_of_cur_chunk)
				{
					const TorrentFile & tf = tor.getFile(r.file);
					QString host = redirected_url.isValid() ? redirected_url.host() : url.host();
					conn->get(host,path + '/' + tf.getPath(),r.off,r.len);
					break;
				}
				length += r.len;
			}
		}
		else
		{
			Uint64 len = (last_chunk - first_chunk) * tor.getChunkSize();
			// last chunk can have a different size
			if (last_chunk == tor.getNumChunks() - 1)
				len += tor.getLastChunkSize();
			else
				len += tor.getChunkSize(); 
			
			QString host = redirected_url.isValid() ? redirected_url.host() : url.host();
			conn->get(host,path,first_chunk * tor.getChunkSize() + bytes_of_cur_chunk,len - bytes_of_cur_chunk);
		}
		chunkStarted(cur_chunk);
	}
	
	void WebSeed::chunkStarted(Uint32 chunk)
	{
		Uint32 csize = cman.getChunk(chunk)->getSize();
		Uint32 pieces_count = csize / MAX_PIECE_LEN;
		if (csize % MAX_PIECE_LEN > 0)
			pieces_count++;
	
		if (!current)
		{
			current = new WebSeedChunkDownload(this,url.prettyUrl(),chunk,pieces_count);
			chunkDownloadStarted(current,chunk);
		}
		else if (current->chunk != chunk)
		{
			chunkStopped();
			current = new WebSeedChunkDownload(this,url.prettyUrl(),chunk,pieces_count);
			chunkDownloadStarted(current,chunk);
		}
	}
	
	void WebSeed::chunkStopped()
	{
		if (current)
		{
			chunkDownloadFinished(current,current->chunk);
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
			readData();
			Out(SYS_CON|LOG_DEBUG) << "WebSeed: connection not OK" << endl;
			// shit happened delete connection
			status = conn->getStatusString();
			if (conn->responseCode() == 404)
			{
				// if not found then retire this webseed
				num_failures = 3;
				status = i18n("Not in use");
			}
			delete conn;
			conn = 0;
			chunkStopped();
			first_chunk = last_chunk = cur_chunk = tor.getNumChunks() + 1;
			num_failures++;
			return 0;
		}
		else if (conn->closed())
		{
			// Make sure we handle all data
			readData();
			
			Out(SYS_CON|LOG_DEBUG) << "WebSeed: connection closed" << endl;
			delete conn;
			conn = 0;
			
			status = i18n("Connection closed");
			chunkStopped();
			if (last_chunk < tor.getNumChunks())
			{
				// lets try this again if we have not yet got the full range
				download(cur_chunk,last_chunk);
				status = conn->getStatusString();
			}
		}
		else if (conn->isRedirected())
		{
			// Make sure we handle all data
			readData();
			redirected(conn->redirectedUrl());
		}
		else 
		{
			readData();
			if (range_queue.count() > 0 && conn->ready())
			{
				if (conn->closed())
				{
					// after a redirect it is possible that the connection is closed
					// so we need to reconnect to the old url
					conn->deleteLater();
					conn = new HttpConnection();
					conn->setGroupIDs(up_gid,down_gid);
					connectToServer();
				}
				
				QString path = url.path();
				if (path.endsWith('/'))
					path += tor.getNameSuggestion();
				
				// ask for the next range
				Range r = range_queue[0];
				range_queue.pop_front();
				const TorrentFile & tf = tor.getFile(r.file);
				QString host = redirected_url.isValid() ? redirected_url.host() : url.host();
				conn->get(host,path + '/' + tf.getPath(),r.off,r.len);
			}
			status = conn->getStatusString();
		}
		
		Uint32 ret = downloaded;
		downloaded = 0;
		total_downloaded += ret;
		return ret;
	}
	
	void WebSeed::readData()
	{
		QByteArray tmp;
		while (conn->getData(tmp) && cur_chunk <= last_chunk)
		{
			//Out(SYS_CON|LOG_DEBUG) << "WebSeed: handleData " << tmp.size() << endl;
			if (!current)
				chunkStarted(cur_chunk);
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
	}
	
	void WebSeed::handleData(const QByteArray & tmp)
	{
//		Out(SYS_GEN|LOG_DEBUG) << "Handling data: " << tmp.length() << " bytes" << endl;
		Uint32 off = 0;
		while (off < (Uint32)tmp.size() && cur_chunk <= last_chunk)
		{
			Chunk* c = cman.getChunk(cur_chunk);
			Uint32 bl = c->getSize() - bytes_of_cur_chunk;
			if (bl > tmp.size() - off)
				bl = tmp.size() - off;
					
			// ignore data if we already have it
			if (c->getStatus() != Chunk::ON_DISK)
			{
				if (!cur_piece || cur_piece->parentChunk() != c)
					cur_piece = c->getPiece(0,c->getSize(),false);

				memcpy(cur_piece->data() + bytes_of_cur_chunk,tmp.data() + off,bl);
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
				if (c->getStatus() != Chunk::ON_DISK)
				{
					chunkReady(c);
				}
				
				chunkStopped();
				cur_piece = PieceDataPtr(0);
				if (cur_chunk <= last_chunk)
				{
					c = cman.getChunk(cur_chunk);
					cur_piece = c->getPiece(0,c->getSize(),false);
					chunkStarted(cur_chunk);
				}
			}
		}
	}

	void WebSeed::fillRangeList(Uint32 chunk)
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
			if (range_queue.count() == 0)
				range_queue.append(r);
			else if (range_queue.back().file != r.file)
				range_queue.append(r);
			else
			{
				// the last range and this one are in the same file
				// so expand it
				Range & l = range_queue.back();
				l.len += r.len;
			}
			
			passed += r.len;
		}
	}

	void WebSeed::onExcluded(Uint32 from,Uint32 to)
	{
		if (from <= first_chunk && first_chunk <= to && from <= last_chunk && last_chunk <= to)
		{
			reset();
		}
	}
	
	void WebSeed::chunkDownloaded(Uint32 chunk)
	{
		// reset if chunk downloaded is in the range we are currently downloading
		if (chunk >= cur_chunk) 
		{
			reset();
		}
	}
	
	void WebSeed::setEnabled(bool on)
	{
		WebSeedInterface::setEnabled(on);
		if (!on)
		{
			reset();
		}
	}
	
	void WebSeed::redirected(const KUrl & to_url)
	{
		delete conn;
		conn = 0;
		if (to_url.isValid() && to_url.protocol() == "http")
		{
			redirected_url = to_url;
			download(cur_chunk,last_chunk);
			status = conn->getStatusString();
		}
		else
		{
			num_failures = 3;
			status = i18n("Not in use");
			cur_chunk = last_chunk = first_chunk = tor.getNumChunks() + 1;
		}
	}

	
	////////////////////////////////////////////
	
	WebSeedChunkDownload::WebSeedChunkDownload(WebSeed* ws,const QString & url,Uint32 index,Uint32 total) 
	: ws(ws),url(url),chunk(index),total_pieces(total),pieces_downloaded(0)
	{}
		 
	WebSeedChunkDownload::~WebSeedChunkDownload()
	{
	}
		
	void WebSeedChunkDownload::getStats(Stats & s)
	{
		s.current_peer_id = url;
		s.chunk_index = chunk;
		s.num_downloaders = 1;
		s.download_speed = ws->getDownloadRate();
		s.pieces_downloaded = pieces_downloaded;
		s.total_pieces = total_pieces;
	}
		

}
#include "webseed.moc"
