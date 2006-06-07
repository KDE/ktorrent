/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson & Maggioni Marcello               *
 *   joris.guisson@gmail.com                                               *
 *   marcello.maggioni@gmail.com                                           *
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

#include <klocale.h>
#include <kapplication.h>
#include <util/log.h>
#include <util/file.h>
#include <util/error.h>
#include <util/array.h>
#include <util/functions.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>
#include <torrent/torrentfile.h>
#include "multidatachecker.h"

namespace bt
{

	MultiDataChecker::MultiDataChecker(): DataChecker()
	{}


	MultiDataChecker::~MultiDataChecker()
	{}


	void MultiDataChecker::check(const QString& path, const Torrent& tor)
	{
		Uint32 num_chunks = tor.getNumChunks();
		// initialize the bitsets
		downloaded = BitSet(num_chunks);
		failed = BitSet(num_chunks);
		
		QString cache = path;
		if (!cache.endsWith(bt::DirSeparator()))
			cache += bt::DirSeparator();
		
		Uint64 chunk_size = tor.getChunkSize();
		Uint32 num_files = 0;
		Uint32 cur_chunk = 0;
		Uint32 bytes_read = 0;
		
		Uint32 last_update_time = bt::GetCurrentTime();
		
		Array<Uint8> buf(chunk_size);
		
		num_files = tor.getNumFiles();
		for (Uint32 CurrentFile = 0;CurrentFile < num_files;CurrentFile++)
		{
			if (listener && listener->needToStop())
				return;
			
			const TorrentFile & tf = tor.getFile(CurrentFile);
			cur_chunk = tf.getFirstChunk();
			bytes_read = tf.getFirstChunkOffset();
			File fptr;
			if (!fptr.open(cache + tf.getPath(), "rb"))
			{
				Out() << QString("Warning : Cannot open %1 : %2").arg(cache + 
				tf.getPath()).arg(fptr.errorString()) << endl;
			}
			else
			{	
				for ( ; cur_chunk <= tf.getLastChunk(); cur_chunk++ )
				{	
					if (listener)
					{
						listener->progress(cur_chunk,num_chunks);
						if (listener->needToStop())
							return;
					}
				
					Uint32 now = bt::GetCurrentTime();
					if (now - last_update_time > 1000)
					{
						Out() << "Checked " << cur_chunk << " chunks" << endl;
						KApplication::kApplication()->processEvents();
						last_update_time = now;
					}

					bytes_read += fptr.read(buf + bytes_read, chunk_size - bytes_read);
					if (bytes_read == chunk_size || cur_chunk + 1 == tor.getNumChunks() )
					{
						SHA1Hash ChunkHash = SHA1Hash::generate(buf, bytes_read );
						bool ok = (ChunkHash == tor.getHash(cur_chunk));
						downloaded.set(cur_chunk,ok);
						failed.set(cur_chunk,!ok);
						bytes_read = 0;
						if (listener)
							listener->status(failed.numOnBits(),downloaded.numOnBits());
					}
				}
			}
				
		}
	}
}
