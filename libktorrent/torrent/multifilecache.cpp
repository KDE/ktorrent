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
#include <util/file.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <util/error.h>
#include <util/log.h>
#include "torrent.h"
#include "cache.h"
#include "multifilecache.h"
#include "globals.h"
#include "chunk.h"
#include <util/cachefile.h>
#include "preallocationthread.h"


namespace bt
{
	static Uint64 FileOffset(Chunk* c,const TorrentFile & f,Uint64 chunk_size);


	MultiFileCache::MultiFileCache(Torrent& tor,const QString & tmpdir,const QString & datadir,bool custom_output_name) : Cache(tor, tmpdir,datadir)
	{
		cache_dir = tmpdir + "cache" + bt::DirSeparator();
		if (datadir.length() == 0)
			this->datadir = guessDataDir();
		if (!custom_output_name)
			output_dir = this->datadir + tor.getNameSuggestion() + bt::DirSeparator();
		else
			output_dir = this->datadir;
		files.setAutoDelete(true);
	}


	MultiFileCache::~MultiFileCache()
	{}
	
	QString MultiFileCache::guessDataDir()
	{
		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			TorrentFile & tf = tor.getFile(i);
			if (tf.doNotDownload())
				continue;
			
			QString p = cache_dir + tf.getPath();
			QFileInfo fi(p);
			if (!fi.isSymLink())
				continue;
			
			QString dst = fi.readLink();
			QString tmp = tor.getNameSuggestion() + bt::DirSeparator() + tf.getPath();
			dst = dst.left(dst.length() - tmp.length());
			if (dst.length() == 0)
				continue;
			
			if (!dst.endsWith(bt::DirSeparator()))
				dst += bt::DirSeparator();
			Out() << "Guessed outputdir to be " << dst << endl;
			return dst;
		}
		
		return QString::null;
	}
	
	QString MultiFileCache::getOutputPath() const
	{
		return output_dir;
	}

	void MultiFileCache::close()
	{
		files.clear();
	}
	
	void MultiFileCache::open()
	{
		// open all files
		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			TorrentFile & tf = tor.getFile(i);
			if (files.contains(i))
				files.erase(i);
			
			CacheFile* fd = new CacheFile();
			try
			{
				fd->open(cache_dir + tf.getPath(),tf.getSize());
				files.insert(i,fd);
			}
			catch (...)
			{
				delete fd;
				fd = 0;
				throw;
			}
		}
	}

	void MultiFileCache::changeDataDir(const QString& ndir)
	{
		Cache::changeTmpDir(ndir);
		cache_dir = tmpdir + "cache/";
	}

	void MultiFileCache::create()
	{
		if (!bt::Exists(cache_dir))
			MakeDir(cache_dir);
		if (!bt::Exists(output_dir))
			MakeDir(output_dir);
		if (!bt::Exists(tmpdir + "dnd"))
			bt::MakeDir(tmpdir + "dnd");

		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			TorrentFile & tf = tor.getFile(i);
			if (tf.doNotDownload())
			{
				touch(tf.getPath(),true);
			}
			else
			{
				touch(tf.getPath(),false);
			}
		}
	}

	void MultiFileCache::touch(const QString fpath,bool dnd)
	{
		// first split fpath by / seperator
		QStringList sl = QStringList::split(bt::DirSeparator(),fpath);
		// create all necessary subdirs
		QString ctmp = cache_dir;
		QString otmp = output_dir;
		QString dtmp = tmpdir + "dnd" + bt::DirSeparator();
		for (Uint32 i = 0;i < sl.count() - 1;i++)
		{
			otmp += sl[i];
			ctmp += sl[i];
			dtmp += sl[i];
			// we need to make the same directory structure in the cache,
			// the output_dir and the dnd directory
			if (!bt::Exists(ctmp))
				MakeDir(ctmp);
			if (!bt::Exists(otmp))
				MakeDir(otmp);
			if (!bt::Exists(dtmp))
				MakeDir(dtmp);
			otmp += bt::DirSeparator();
			ctmp += bt::DirSeparator();
			dtmp += bt::DirSeparator();
		}

		// then make the file
		QString tmp = dnd ? tmpdir + "dnd" + bt::DirSeparator() : output_dir;
		if (!bt::Exists(tmp + fpath))
			bt::Touch(tmp + fpath);
		
		// and make a symlink in the cache to it
		if (!bt::Exists(cache_dir + fpath))
			bt::SymLink(tmp + fpath,cache_dir + fpath);
	}

	void MultiFileCache::load(Chunk* c)
	{
		QValueList<Uint32> tflist;
		tor.calcChunkPos(c->getIndex(),tflist);
		
		// one file is simple, just mmap it
		if (tflist.count() == 1)
		{
			const TorrentFile & f = tor.getFile(tflist.first());
			CacheFile* fd = files.find(tflist.first());
			Uint64 off = FileOffset(c,f,tor.getChunkSize());
			Uint8* buf = (Uint8*)fd->map(c,off,c->getSize(),CacheFile::READ);
			if (buf)
				c->setData(buf,Chunk::MMAPPED);
			return;
		}
		
		Uint8* data = new Uint8[c->getSize()];
		Uint64 read = 0; // number of bytes read
		for (Uint32 i = 0;i < tflist.count();i++)
		{
			const TorrentFile & f = tor.getFile(tflist[i]);
			CacheFile* fd = files.find(tflist[i]);
			
			// first calculate offset into file
			// only the first file can have an offset
			// the following files will start at the beginning
			Uint64 off = 0;
			if (i == 0)
				off = FileOffset(c,f,tor.getChunkSize());
			
			Uint32 to_read = 0;
			// then the amount of data we can read from this file
			if (tflist.count() == 1)
				to_read = c->getSize();
			else if (i == 0)
				to_read = f.getLastChunkSize();
			else if (i == tflist.count() - 1)
				to_read = c->getSize() - read;
			else
				to_read = f.getSize();
			
		
			// read part of data
			fd->read(data + read,to_read,off);
			read += to_read;
		}
		c->setData(data,Chunk::BUFFERED);
	}

	
	bool MultiFileCache::prep(Chunk* c)
	{
		// find out in which files a chunk lies
		QValueList<Uint32> tflist;
		tor.calcChunkPos(c->getIndex(),tflist);
		
//		Out() << "Prep " << c->getIndex() << endl;
		if (tflist.count() == 1)
		{
			// in one so just mmap it
			Uint64 off = FileOffset(c,tor.getFile(tflist.first()),tor.getChunkSize());
			CacheFile* fd = files.find(tflist.first());
			Uint8* buf = (Uint8*)fd->map(c,off,c->getSize(),CacheFile::RW);
			if (!buf)
			{
				// if mmap fails use buffered mode
				Out() << "Warning : mmap failed, falling back to buffered mode" << endl;
				c->allocate();
				c->setStatus(Chunk::BUFFERED);
			}
			else
			{
				c->setData(buf,Chunk::MMAPPED);
			}
		}
		else
		{
			// just allocate it
			c->allocate();
			c->setStatus(Chunk::BUFFERED);
		}
		return true;
	}

	void MultiFileCache::save(Chunk* c)
	{
		QValueList<Uint32> tflist;
		tor.calcChunkPos(c->getIndex(),tflist);
		
		if (c->getStatus() == Chunk::MMAPPED)
		{
			// mapped chunks are easy
			CacheFile* fd = files.find(tflist[0]);
			fd->unmap(c->getData(),c->getSize());
			c->clear();
			c->setStatus(Chunk::ON_DISK);
			return;
		}
		else if (tflist.count() == 0 && c->getStatus() == Chunk::BUFFERED)
		{
			// buffered chunks are slightly more difficult
			CacheFile* fd = files.find(tflist[0]);
			Uint64 off = c->getIndex() * tor.getChunkSize();
			fd->write(c->getData(),c->getSize(),off);
			c->clear();
			c->setStatus(Chunk::ON_DISK);
			return;
		}
		
	//	Out() << "Writing to " << tflist.count() << " files " << endl;
		Uint64 written = 0; // number of bytes written
		for (Uint32 i = 0;i < tflist.count();i++)
		{
			const TorrentFile & f = tor.getFile(tflist[i]);
			CacheFile* fd = files.find(tflist[i]);
			

			// first calculate offset into file
			// only the first file can have an offset
			// the following files will start at the beginning
			Uint64 off = 0;
			Uint32 to_write = 0;
			if (i == 0)
			{
				off = FileOffset(c,f,tor.getChunkSize());
			}

			// the amount of data we can write to this file
			if (tflist.count() == 1)
				to_write = c->getSize();
			else if (i == 0)
				to_write = f.getLastChunkSize();
			else if (i == tflist.count() - 1)
				to_write = c->getSize() - written;
			else
				to_write = f.getSize();
			
		//	Out() << "to_write " << to_write << endl;
			// write the data
			fd->write(c->getData() + written,to_write,off);
			written += to_write;
		}
		
		// set the chunk to on disk and clear it
		c->clear();
		c->setStatus(Chunk::ON_DISK);
	}
	
	void MultiFileCache::downloadStatusChanged(TorrentFile* tf, bool download)
	{
		bool dnd = !download;
		CacheFile* fd = files.find(tf->getIndex());
		
		QString dnd_dir = tmpdir + "dnd" + bt::DirSeparator();
		// if it is dnd and it is already in the dnd tree do nothing
		if (dnd && bt::Exists(dnd_dir + tf->getPath()))
			return;
		
		// if it is !dnd and it is already in the output_dir tree do nothing
		if (!dnd && bt::Exists(output_dir + tf->getPath()))
			return;
		if (fd)
			fd->close(true);
		
		try
		{
			// now move it from output_dir tree to dnd tree or vica versa
			// delete the symlink
			bt::Delete(cache_dir + tf->getPath());
			if (dnd)
			{
				bt::Move(output_dir + tf->getPath(),dnd_dir + tf->getPath());
				bt::SymLink(dnd_dir + tf->getPath(),cache_dir + tf->getPath());
			}
			else
			{
				bt::Move(dnd_dir + tf->getPath(),output_dir + tf->getPath());
				bt::SymLink(output_dir + tf->getPath(),cache_dir + tf->getPath());
			}
			
		}
		catch (bt::Error & e)
		{
			Out() << e.toString() << endl;
		}
		
		if (fd)
			fd->open(cache_dir + tf->getPath(),tf->getSize());
	}
	
	void MultiFileCache::preallocateDiskSpace(PreallocationThread* pt)
	{
		Out() << "MultiFileCache::preallocateDiskSpace" << endl;
		PtrMap<Uint32,CacheFile>::iterator i = files.begin();
		while (i != files.end() && !pt->isStopped())
		{
			CacheFile* cf = i->second;
			cf->preallocate(pt);
			i++;
		}
	}
	
	///////////////////////////////

	Uint64 FileOffset(Chunk* c,const TorrentFile & f,Uint64 chunk_size)
	{
		Uint64 off = 0;
		if (c->getIndex() - f.getFirstChunk() > 0)
			off = (c->getIndex() - f.getFirstChunk() - 1) * chunk_size;
		if (c->getIndex() > 0)
			off += (chunk_size - f.getFirstChunkOffset());
		return off;
	}
	
	
}

