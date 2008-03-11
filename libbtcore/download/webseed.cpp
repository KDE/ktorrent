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
#include <util/log.h>
#include <torrent/torrent.h>
#include <diskio/chunkmanager.h>
#include "webseed.h"
#include "httpconnection.h"

namespace bt
{

	WebSeed::WebSeed(const KUrl & url,const Torrent & tor,ChunkManager & cman) : url(url),tor(tor),cman(cman)
	{
		first_chunk = last_chunk = tor.getNumChunks() + 1;
		num_failures = 0;
		conn = 0;
		downloaded = 0;
	}


	WebSeed::~WebSeed()
	{
		delete conn;
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
		if (path.endsWith("/"))
			path += tor.getNameSuggestion();
		
		// open connection and connect if needed
		if (!conn)
			conn = new HttpConnection();
		
		if (!conn->connected())
			conn->connectTo(url);
		
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
				conn->get(url.host(),path + "/" + tf.getPath(),r.off,r.len);
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
			num_failures++;
			if (num_failures < 3)
			{
				// lets try this again
				download(cur_chunk,last_chunk);
			}
			return 0;
		}
		else if (conn->closed())
		{
			Out(SYS_CON|LOG_DEBUG) << "WebSeed: connection closed" << endl;
			delete conn;
			conn = 0;
			// lets try this again
			download(cur_chunk,last_chunk);
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
			
			Uint32 ret = downloaded;
			downloaded = 0;
			return ret;
		}
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
			if (bytes_of_cur_chunk == c->getSize())
			{
				// we have one ready
				bytes_of_cur_chunk = 0;
				cur_chunk++;
				if (c->getStatus() == Chunk::BUFFERED || c->getStatus() == Chunk::MMAPPED)
					chunkReady(c);
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
