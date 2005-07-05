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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <qstringlist.h>
#include <qfileinfo.h>
#include <klocale.h>
#include <libutil/file.h>
#include "torrent.h"
#include "cache.h"
#include "multifilecache.h"
#include <libutil/error.h>
#include "chunk.h"
#include <libutil/fileops.h>
#include <libutil/functions.h>


namespace bt
{
	/*
		It's possible for a Chunk to lie in two files.
		file2 should be set to QString::null if it isn't.
	*/
	struct ChunkPos
	{
		QString file1;
		Uint32 off1;
		Uint32 size1;

		QString file2;
		Uint32 off2;
	};

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
			Torrent::File f;
			tor.getFile(i,f);
			touch(f.path);
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

	void MultiFileCache::calcChunkPos(Chunk* c,ChunkPos & pos)
	{
		// calculate in which file c lies and at what position

		// the number of bytes we have allready passed
		Uint32 bytes_passed = 0;
		// The byte offset the chunk begins
		Uint32 coff = c->getIndex() * tor.getChunkSize();
		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			Torrent::File f;
			tor.getFile(i,f);

			if (coff < bytes_passed + f.size)
			{
				// bingo we found it
				if (coff + c->getSize() <= bytes_passed + f.size)
				{
					// The chunk is in 1 file
					pos.file1 = f.path;
					pos.off1 = coff - bytes_passed;
					pos.size1 = c->getSize();
					pos.file2 = QString::null;
					return;
				}
				else
				{
					// The chunk is in 2 files

					// check if the next files exists
					if (i + 1 >= tor.getNumFiles())
						throw Error(i18n("Cannot find chunk"));

					// get the second file
					Torrent::File f2;
					tor.getFile(i+1,f2);
					
					pos.file1 = f.path;
					pos.off1 = coff - bytes_passed;
					pos.size1 = c->getSize() - ((coff + c->getSize()) - (bytes_passed + f.size));
					pos.file2 = f2.path;
					pos.off2 = 0;
					return;
				}
			}
			else
			{
				// not found move to next file
				bytes_passed += f.size;
			}
		}

		// if we get here, we're in serious trouble
		throw Error(i18n("Cannot find chunk"));
	}

	void MultiFileCache::load(Chunk* c)
	{
		ChunkPos cp;
		calcChunkPos(c,cp);

		if (cp.file2.isNull())
		{
			File fptr;
			if (!fptr.open(cache_dir + cp.file1,"rb"))
				throw Error(i18n("Cannot open file %1: %2")
						.arg(cp.file1).arg(fptr.errorString()));

			fptr.seek(File::BEGIN,cp.off1);
			Uint8* data = new Uint8[c->getSize()];
			fptr.read(data,c->getSize());
			c->setData(data);
		}
		else
		{
			File fptr1;
			if (!fptr1.open(cache_dir + cp.file1,"rb"))
				throw Error(i18n("Cannot open file %1: %2")
						.arg(cp.file1).arg(fptr1.errorString()));

			File fptr2;
			if (!fptr2.open(cache_dir + cp.file2,"rb"))
				throw Error(i18n("Cannot open file %1: %2")
						.arg(cp.file2).arg(fptr2.errorString()));

			Uint8* data = new Uint8[c->getSize()];
			
			// read first part of chunk from fptr1
			fptr1.seek(File::BEGIN,cp.off1);
			fptr1.read(data,cp.size1);
			// read second part from fptr2
			fptr2.read(data + cp.size1,c->getSize() - cp.size1);
			c->setData(data);
		}
	}

	void MultiFileCache::save(Chunk* c)
	{
		ChunkPos cp;
		calcChunkPos(c,cp);

		if (cp.file2.isNull())
			saveChunkOneFile(c,cp);
		else
			saveChunkTwoFiles(c,cp);

		Uint32 chunk_pos = c->getIndex() * tor.getChunkSize();
		// set the offset and clear the chunk
		c->setCacheFileOffset(chunk_pos);
		c->clear();
	}

	void MultiFileCache::saveChunkOneFile(Chunk* c,ChunkPos & pos)
	{
		File fptr;
		if (!fptr.open(cache_dir + pos.file1,"r+b"))
			throw Error("Can't open cache file");
			
		// jump to end of file
		fptr.seek(File::END,0);
		unsigned int cache_size = fptr.tell();
	
		// see if the cache is big enough for the chunk
		if (pos.off1 <= cache_size)
		{
			// big enough so just jump to the right posiition and write
			// the chunk
			fptr.seek(File::BEGIN,pos.off1);
			fptr.write(c->getData(),pos.size1);
		}
		else
		{
			// write random shit to enlarge the file
			Uint32 num_empty_bytes = pos.off1 - cache_size;
			Uint8 b[1024];
			Uint32 nw = 0;
			while (nw < num_empty_bytes)
			{
				Uint32 left = num_empty_bytes - nw;
				fptr.write(b,left < 1024 ? left : 1024);
				nw += 1024;
			}
	
			// now write the chunks at the good position
			fptr.seek(File::BEGIN,pos.off1);
			fptr.write(c->getData(),pos.size1);
		}
	}

	void MultiFileCache::saveChunkTwoFiles(Chunk* c,ChunkPos & pos)
	{
		// first save to first file
		saveChunkOneFile(c,pos);
		// open second and save second piece of chunk
		File fptr;
		if (!fptr.open(cache_dir + pos.file2,"r+b"))
			throw Error("Can't open cache file");

		fptr.write(c->getData() + pos.size1,c->getSize() - pos.size1);
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
}
