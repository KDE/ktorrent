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
#include <util/fileops.h>
#include <util/error.h>
#include <util/array.h>
#include <util/functions.h>
#include <torrent/dndfile.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>
#include <torrent/torrentfile.h>
#include "multidatachecker.h"

namespace bt
{

	MultiDataChecker::MultiDataChecker(): DataChecker(),buf(0)
	{}


	MultiDataChecker::~MultiDataChecker()
	{
		delete [] buf;
	}
	
	void MultiDataChecker::check(const QString& path, const Torrent& tor,const QString & dnddir)
	{
		Uint32 num_chunks = tor.getNumChunks();
		// initialize the bitsets
		downloaded = BitSet(num_chunks);
		failed = BitSet(num_chunks);
		
		cache = path;
		if (!cache.endsWith(bt::DirSeparator()))
			cache += bt::DirSeparator();
		
		dnd_dir = dnddir;
		if (!dnddir.endsWith(bt::DirSeparator()))
			dnd_dir += bt::DirSeparator();
		
		Uint64 chunk_size = tor.getChunkSize();
		Uint32 cur_chunk = 0;
		TimeStamp last_update_time = bt::GetCurrentTime();
		
		buf = new Uint8[chunk_size];
		
		for (cur_chunk = 0;cur_chunk < num_chunks;cur_chunk++)
		{
			Uint32 cs = (cur_chunk == num_chunks - 1) ? tor.getFileLength() % chunk_size : chunk_size;
			if (cs == 0)
				cs = chunk_size;
			if (!loadChunk(cur_chunk,cs,tor))
			{
				downloaded.set(cur_chunk,false);
				failed.set(cur_chunk,true);
				continue;
			}
			
			bool ok = (SHA1Hash::generate(buf,cs) == tor.getHash(cur_chunk));
			downloaded.set(cur_chunk,ok);
			failed.set(cur_chunk,!ok);
			
			if (listener)
			{
				listener->status(failed.numOnBits(),downloaded.numOnBits());
				listener->progress(cur_chunk,num_chunks);
				if (listener->needToStop())
					return;
			}
			
			TimeStamp now = bt::GetCurrentTime();
			if (now - last_update_time > 1000)
			{
				Out() << "Checked " << cur_chunk << " chunks" << endl;
			//	KApplication::kApplication()->processEvents();
				last_update_time = now;
			}
		}	
	}
	
	static Uint32 ReadFullChunk(Uint32 chunk,Uint32 cs,
								const TorrentFile & tf,
								const Torrent & tor,
								Uint8* buf,
								const QString & cache)
	{
		File fptr;
		if (!fptr.open(cache + tf.getPath(), "rb"))
		{
			Out() << QString("Warning : Cannot open %1 : %2").arg(cache + 
					tf.getPath()).arg(fptr.errorString()) << endl;
			return 0;
		}
		
		Uint64 off = tf.fileOffset(chunk,tor.getChunkSize());
		fptr.seek(File::BEGIN,off);
		return fptr.read(buf,cs);
	}
	
	bool MultiDataChecker::loadChunk(Uint32 ci,Uint32 cs,const Torrent & tor)
	{
		QValueList<Uint32> tflist;
		tor.calcChunkPos(ci,tflist);
		
		// one file is simple
		if (tflist.count() == 1)
		{
			const TorrentFile & f = tor.getFile(tflist.first());
			if (!f.doNotDownload())
			{
				ReadFullChunk(ci,cs,f,tor,buf,cache);
				return true;
			}
			return false;
		}
		
		Uint64 read = 0; // number of bytes read
		for (Uint32 i = 0;i < tflist.count();i++)
		{
			const TorrentFile & f = tor.getFile(tflist[i]);
				
			// first calculate offset into file
			// only the first file can have an offset
			// the following files will start at the beginning
			Uint64 off = 0;
			if (i == 0)
				off = f.fileOffset(ci,tor.getChunkSize());
			
			Uint32 to_read = 0;
			// then the amount of data we can read from this file
			if (i == 0)
				to_read = f.getLastChunkSize();
			else if (i == tflist.count() - 1)
				to_read = cs - read;
			else
				to_read = f.getSize();
			
			// read part of data
			if (f.doNotDownload())
			{
				if (!dnd_dir.isNull() && bt::Exists(dnd_dir + f.getPath() + ".dnd"))
				{
					Uint32 ret = 0;
					DNDFile dfd(dnd_dir + f.getPath() + ".dnd");
					if (i == 0)
						ret = dfd.readLastChunk(buf,read,cs);
					else if (i == tflist.count() - 1)
						ret = dfd.readFirstChunk(buf,read,cs);
					else
						ret = dfd.readFirstChunk(buf,read,cs);
					
					if (ret > 0 && ret != to_read)
						Out() << "Warning : MultiDataChecker::load ret != to_read (dnd)" << endl;
				}
			}
			else
			{
				if (!bt::Exists(cache + f.getPath()) || bt::FileSize(cache + f.getPath()) < off)
					return false;
				
				File fptr;
				if (!fptr.open(cache + f.getPath(), "rb"))
				{
					Out() << QString("Warning : Cannot open %1 : %2").arg(cache + 
							f.getPath()).arg(fptr.errorString()) << endl;
					return false;
				}
				else
				{
					fptr.seek(File::BEGIN,off);
					if (fptr.read(buf+read,to_read) != to_read)
						Out() << "Warning : MultiDataChecker::load ret != to_read" << endl;
				}
			}
			read += to_read;
		}
		return true;
	}
}
