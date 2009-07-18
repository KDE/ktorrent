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
#include "multidatachecker.h"
#include <klocale.h>
#include <util/log.h>
#include <util/file.h>
#include <util/fileops.h>
#include <util/error.h>
#include <util/array.h>
#include <util/functions.h>
#include <diskio/dndfile.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>
#include <torrent/torrentfile.h>

namespace bt
{

	MultiDataChecker::MultiDataChecker(): DataChecker(),buf(0)
	{}


	MultiDataChecker::~MultiDataChecker()
	{
		delete [] buf;
	}
	
	void MultiDataChecker::check(const QString& path, const Torrent& tor,const QString & dnddir,const BitSet & status)
	{
		Uint32 num_chunks = tor.getNumChunks();
		// initialize the bitset
		result = BitSet(num_chunks);
		
		cache = path;
		if (!cache.endsWith(bt::DirSeparator()))
			cache += bt::DirSeparator();
		
		dnd_dir = dnddir;
		if (!dnddir.endsWith(bt::DirSeparator()))
			dnd_dir += bt::DirSeparator();
		
		Uint64 chunk_size = tor.getChunkSize();
		Uint32 cur_chunk = 0;
		buf = new Uint8[chunk_size];
		
		for (cur_chunk = 0;cur_chunk < num_chunks;cur_chunk++)
		{
			Uint32 cs = (cur_chunk == num_chunks - 1) ? tor.getLastChunkSize() : chunk_size;
			if (cs == 0)
				cs = chunk_size;
			if (!loadChunk(cur_chunk,cs,tor))
			{
				if (status.get(cur_chunk))
					failed++;
				else
					not_downloaded++;
				continue;
			}
			
			bool ok = (SHA1Hash::generate(buf,cs) == tor.getHash(cur_chunk));
			result.set(cur_chunk,ok);
			if (ok && status.get(cur_chunk))
				downloaded++;
			else if (!ok && status.get(cur_chunk))
				failed++;
			else if (!ok && !status.get(cur_chunk))
				not_downloaded++;
			else if (ok && !status.get(cur_chunk))
				found++;
			
			if (listener)
			{
				listener->status(failed,found,downloaded,not_downloaded);
				listener->progress(cur_chunk,num_chunks);
				if (listener->needToStop())
					return;
			}
		}	
	}
	
	static Uint32 ReadFullChunk(Uint32 chunk,Uint32 cs,
								const TorrentFile & tf,
								const Torrent & tor,
								Uint8* buf)
	{
		File fptr;
		if (!fptr.open(tf.getPathOnDisk(), "rb"))
		{
			Out(SYS_GEN|LOG_DEBUG) << QString("Warning : Cannot open %1 : %2").arg(tf.getPathOnDisk()).arg(fptr.errorString()) << endl;
			return 0;
		}
		
		Uint64 off = tf.fileOffset(chunk,tor.getChunkSize());
		fptr.seek(File::BEGIN,off);
		return fptr.read(buf,cs);
	}
	
	bool MultiDataChecker::loadChunk(Uint32 ci,Uint32 cs,const Torrent & tor)
	{
		QList<Uint32> tflist;
		tor.calcChunkPos(ci,tflist);
		
		// one file is simple
		if (tflist.count() == 1)
		{
			const TorrentFile & f = tor.getFile(tflist.first());
			if (!f.doNotDownload())
			{
				ReadFullChunk(ci,cs,f,tor,buf);
				return true;
			}
			return false;
		}
		
		Uint64 read = 0; // number of bytes read
		for (int i = 0;i < tflist.count();i++)
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
				QString dnd_path = QString("file%1.dnd").arg(f.getIndex());
				QString dnd_file = dnd_dir + dnd_path;
				if (!bt::Exists(dnd_file)) // could be an old style dnd dir
					dnd_file = dnd_dir + f.getUserModifiedPath() + ".dnd";
				
				if (bt::Exists(dnd_file))
				{
					Uint32 ret = 0;
					DNDFile dfd(dnd_file,&f,tor.getChunkSize());
					if (i == 0)
						ret = dfd.readLastChunk(buf + read,0,to_read);
					else
						ret = dfd.readFirstChunk(buf + read,0,to_read);
					
					if (ret > 0 && ret != to_read)
						Out(SYS_GEN|LOG_DEBUG) << "Warning : MultiDataChecker::load ret != to_read (dnd)" << endl;
				}
			}
			else
			{
				if (!bt::Exists(f.getPathOnDisk()) || bt::FileSize(f.getPathOnDisk()) < off)
					return false;
				
				File fptr;
				if (!fptr.open(f.getPathOnDisk(), "rb"))
				{
					Out(SYS_GEN|LOG_DEBUG) << QString("Warning : Cannot open %1 : %2").arg(f.getPathOnDisk()).arg(fptr.errorString()) << endl;
					return false;
				}
				else
				{
					fptr.seek(File::BEGIN,off);
					if (fptr.read(buf+read,to_read) != to_read)
						Out(SYS_GEN|LOG_DEBUG) << "Warning : MultiDataChecker::load ret != to_read" << endl;
				}
			}
			read += to_read;
		}
		return true;
	}
}
