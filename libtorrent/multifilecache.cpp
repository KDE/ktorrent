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
#include <qstringlist.h>
#include <qfileinfo.h>
#include <klocale.h>
#include <libutil/file.h>
#include <libutil/fileops.h>
#include <libutil/functions.h>
#include <libutil/error.h>
#include <libutil/log.h>
#include "torrent.h"
#include "cache.h"
#include "multifilecache.h"
#include "globals.h"
#include "chunk.h"


namespace bt
{
	static Uint32 FileOffset(Chunk* c,const TorrentFile & f,Uint32 chunk_size);


	MultiFileCache::MultiFileCache(Torrent& tor, const QString& data_dir)
		: Cache(tor, data_dir)
	{
		cache_dir = data_dir + "cache/";
	}


	MultiFileCache::~MultiFileCache()
	{}


	void MultiFileCache::changeDataDir(const QString& ndir)
	{
		Cache::changeDataDir(ndir);
		cache_dir = data_dir + "cache/";
	}

	void MultiFileCache::create()
	{
		if (!bt::Exists(cache_dir))
			MakeDir(cache_dir);

		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			TorrentFile f;
			tor.getFile(i,f);
			touch(f.getPath());
		}
	}

	

	void MultiFileCache::touch(const QString fpath)
	{
		// first split fpath by / seperator
		
		QStringList sl = QStringList::split(bt::DirSeparator(),fpath);

		// create all necessary subdirs
		QString tmp = cache_dir;
		for (Uint32 i = 0;i < sl.count() - 1;i++)
		{
			tmp += sl[i];
			QFileInfo finfo(tmp);
			
			if (!bt::Exists(tmp))
				MakeDir(tmp);
			tmp += bt::DirSeparator();
		}

		// then make the file
		File fptr;
		fptr.open(cache_dir + fpath,"wb");
	}

	void MultiFileCache::load(Chunk* c)
	{
		QValueList<TorrentFile> files;
		tor.calcChunkPos(c->getIndex(),files);
		
		Uint8* data = new Uint8[c->getSize()];
		Uint32 read = 0; // number of bytes read
		for (Uint32 i = 0;i < files.count();i++)
		{
			const TorrentFile & f = files[i];
			File fptr;
			if (!fptr.open(cache_dir + f.getPath(),"rb"))
			{
				delete [] data;
				throw Error(i18n("Cannot open file %1: %2")
						.arg(f.getPath()).arg(fptr.errorString()));
			}

			// first calculate offset into file
			// only the first file can have an offset
			// the following files will start at the beginning
			Uint32 off = 0;
			if (i == 0)
				off = FileOffset(c,f,tor.getChunkSize());
			
			Uint32 to_read = 0;
			// then the amount of data we can read from this file
			if (files.count() == 1)
				to_read = c->getSize();
			else if (i == 0)
				to_read = f.getLastChunkSize();
			else if (i == files.count() - 1)
				to_read = c->getSize() - read;
			else
				to_read = f.getSize();
						
			// read part of data
			fptr.seek(File::BEGIN,off);
			fptr.read(data + read,to_read);
			read += to_read;
		}
		c->setData(data);
	}

	

	void MultiFileCache::save(Chunk* c)
	{
		QValueList<TorrentFile> files;
		tor.calcChunkPos(c->getIndex(),files);

	//	Out() << "Saving " << c->getIndex() << " to " << files.count() << " files" << endl;
		Uint32 written = 0; // number of bytes written
		for (Uint32 i = 0;i < files.count();i++)
		{
			const TorrentFile & f = files[i];
			File fptr;
			if (!fptr.open(cache_dir + f.getPath(),"r+b"))
			{
				throw Error(i18n("Cannot open file %1: %2")
						.arg(f.getPath()).arg(fptr.errorString()));
			}

			// first calculate offset into file
			// only the first file can have an offset
			// the following files will start at the beginning
			Uint32 off = 0;
			Uint32 to_write = 0;
			if (i == 0)
			{
				off = FileOffset(c,f,tor.getChunkSize());

		//		Out() << "off = " << off << endl;
				// we may need to expand the first file
				fptr.seek(File::END,0);
				Uint32 cache_size = fptr.tell();
				if (cache_size < off)
				{
					// write random shit to enlarge the file
					Uint32 num_empty_bytes = off - cache_size + 1;
					Uint8 b[1024];
					Uint32 nw = 0;
					while (nw < num_empty_bytes)
					{
						Uint32 left = num_empty_bytes - nw;
						fptr.write(b,left < 1024 ? left : 1024);
						nw += 1024;
					}
				}
			}

			// the amount of data we can write to this file
			if (files.count() == 1)
				to_write = c->getSize();
			else if (i == 0)
				to_write = f.getLastChunkSize();
			else if (i == files.count() - 1)
				to_write = c->getSize() - written;
			else
				to_write = f.getSize();
			
		//	Out() << "to_write " << to_write << endl;
			// read part of data
			fptr.seek(File::BEGIN,off);
			fptr.write(c->getData() + written,to_write);
			written += to_write;

			fptr.close();
		}
		

		Uint32 chunk_pos = c->getIndex() * tor.getChunkSize();
		// set the offset and clear the chunk
		c->setCacheFileOffset(chunk_pos);
		c->clear();
	}

	void MultiFileCache::saveData(const QString & dir)
	{
		QString d = dir;
		if (!d.endsWith(bt::DirSeparator()))
			d += bt::DirSeparator();

		// first move file
		QString ndir = d + tor.getNameSuggestion();
		Move(cache_dir,ndir);

		// create symlink in data dir
		SymLink(ndir,cache_dir.mid(0,cache_dir.length() - 1));
	}

	bool MultiFileCache::hasBeenSaved() const
	{
		QFileInfo fi(cache_dir.mid(0,cache_dir.length() - 1));
		return fi.isSymLink();
	}

	///////////////////////////////

	Uint32 FileOffset(Chunk* c,const TorrentFile & f,Uint32 chunk_size)
	{
		Uint32 off = 0;
		if (c->getIndex() - f.getFirstChunk() > 0)
			off = (c->getIndex() - f.getFirstChunk() - 1) * chunk_size;
		if (c->getIndex() > 0)
			off += (c->getSize() - f.getFirstChunkOffset());
		return off;
	}
}
