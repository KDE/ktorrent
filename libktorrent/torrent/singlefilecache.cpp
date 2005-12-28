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
#include <klocale.h>
#include <qfileinfo.h>
#include <util/fileops.h>
#include <util/cachefile.h>
#include "singlefilecache.h"
#include <util/error.h>
#include <util/functions.h>
#include <util/log.h>
#include "torrent.h"
#include "chunk.h"
#include "globals.h"


namespace bt
{

	SingleFileCache::SingleFileCache(Torrent& tor,const QString & tmpdir,const QString & datadir)
	: Cache(tor,tmpdir,datadir),fd(0)
	{
		cache_file = tmpdir + "cache";
	}


	SingleFileCache::~SingleFileCache()
	{}

	void SingleFileCache::changeDataDir(const QString & ndir)
	{
		Cache::changeTmpDir(ndir);
		cache_file = tmpdir + "cache";
	}
	
	bool SingleFileCache::prep(Chunk* c)
	{
		if (c->getStatus() != Chunk::NOT_DOWNLOADED)
		{
			Out() << "Warning : can only prep NOT_DOWNLOADED chunks !" << endl;
			return false;
		}
		Uint64 off = c->getIndex() * tor.getChunkSize();
		Uint8* buf = (Uint8*)fd->map(c,off,c->getSize(),CacheFile::RW);
		if (!buf)
		{
			// buffer it if mmapping fails
			Out() << "Warning : mmap failure, falling back to buffered mode" << endl;
			c->allocate();
		}
		else
		{
			c->setData(buf,Chunk::MMAPPED);
		}
		return true;
	}

	void SingleFileCache::load(Chunk* c)
	{
		Uint64 off = c->getIndex() * tor.getChunkSize();
		Uint8* buf = (Uint8*)fd->map(c,off,c->getSize(),CacheFile::READ);
		if (!buf)
			throw Error(i18n("Cannot load chunk %1").arg(c->getIndex()));
		c->setData(buf,Chunk::MMAPPED);
	}

	void SingleFileCache::save(Chunk* c)
	{
		// unmap the chunk if it is mapped
		if (c->getStatus() == Chunk::MMAPPED)
		{
			fd->unmap(c->getData(),c->getSize());
			c->clear();
			c->setStatus(Chunk::ON_DISK);
		}
		else if (c->getStatus() == Chunk::BUFFERED)
		{
			Uint64 off = c->getIndex() * tor.getChunkSize();
			fd->write(c->getData(),c->getSize(),off);
			c->clear();
			c->setStatus(Chunk::ON_DISK);
		}
	}

	void SingleFileCache::create()
	{
		if (!bt::Exists(datadir + tor.getNameSuggestion()))
		{
			QString out_file = datadir + tor.getNameSuggestion();
			if (!bt::Exists(out_file))
				bt::Touch(out_file);
			if (!bt::Exists(cache_file))
				bt::SymLink(out_file,cache_file);
		}
	}
	
	void SingleFileCache::close()
	{
		if (fd)
		{
			fd->close(false);
			delete fd;
			fd = 0;
		}
	}
	
	void SingleFileCache::open()
	{	
		try
		{
			fd = new CacheFile();
			fd->open(cache_file,tor.getFileLength());
		}
		catch (...)
		{
			fd->close(false);
			delete fd;
			fd = 0;
			throw;
		}
	}

}
