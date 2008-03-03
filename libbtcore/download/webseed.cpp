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
#include <torrent/torrent.h>
#include <diskio/chunkmanager.h>
#include "webseed.h"
#include "httpconnection.h"

namespace bt
{

	WebSeed::WebSeed(const KUrl & url,const Torrent & tor,ChunkManager & cman) : url(url),tor(tor),cman(cman)
	{
		first_chunk = last_chunk = tor.getNumChunks() + 1;
	}


	WebSeed::~WebSeed()
	{
	}

	bool WebSeed::busy() const
	{
		return first_chunk < tor.getNumChunks();
	}
		
	void WebSeed::download(Uint32 first,Uint32 last)
	{
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
				conn->get(path + "/" + tf.getPath(),r.off,r.len);
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
			conn->get(path,first_chunk * tor.getChunkSize(),len);
		}
	}
		
	void WebSeed::update()
	{
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

}
#include "webseed.moc"
