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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <QTextStream>
#include <klocale.h>
#include <qfileinfo.h>
#include <qstringlist.h> 
#include <kio/copyjob.h>
#include <util/fileops.h>
#include <util/error.h>
#include <util/functions.h>
#include <util/log.h>
#include <torrent/torrent.h>
#include "chunk.h"
#include "cachefile.h"
#include "singlefilecache.h"
#include "preallocationthread.h"


namespace bt
{

	SingleFileCache::SingleFileCache(Torrent& tor,const QString & tmpdir,const QString & datadir)
	: Cache(tor,tmpdir,datadir),fd(0)
	{
		cache_file = tmpdir + "cache";
		QFileInfo fi(cache_file);
		if (fi.isSymLink()) // old style symlink
			output_file = fi.readLink();
		else
			output_file = datadir + tor.getNameSuggestion();
	}


	SingleFileCache::~SingleFileCache()
	{}
	
	void SingleFileCache::loadFileMap()
	{
		QString file_map = tmpdir + "file_map";
		if (!bt::Exists(file_map))
		{
			saveFileMap();
			return;
		}
		
		QFile fptr(file_map);
		if (!fptr.open(QIODevice::ReadOnly))
			throw Error(i18n("Failed to open %1 : %2",file_map,fptr.errorString()));
		
		output_file = QString::fromLocal8Bit(fptr.readLine().trimmed());
	}

	void SingleFileCache::saveFileMap()
	{
		QString file_map = tmpdir + "file_map";
		QFile fptr(file_map);
		if (!fptr.open(QIODevice::WriteOnly))
			throw Error(i18n("Failed to create %1 : %2",file_map,fptr.errorString()));
		
		QTextStream out(&fptr);
		out << output_file << ::endl;
	}
	
	void SingleFileCache::changeTmpDir(const QString & ndir)
	{
		Cache::changeTmpDir(ndir);
		cache_file = tmpdir + "cache";
	}
	
	void SingleFileCache::changeOutputPath(const QString & outputpath)
	{
		close();
		output_file = outputpath;
		datadir = output_file.left(output_file.lastIndexOf(bt::DirSeparator()));
		saveFileMap();
	}
	
	KJob* SingleFileCache::moveDataFiles(const QString & ndir)
	{
		return KIO::move(output_file,ndir);
	}
	
	void SingleFileCache::moveDataFilesFinished(KJob* job)
	{
		Q_UNUSED(job);
	}
	
	bool SingleFileCache::prep(Chunk* c)
	{
		if (mmap_failures >= 3)
		{
			// mmap continuously fails, so stop using it
			c->allocate();
			c->setStatus(Chunk::BUFFERED);
		}
		else
		{
			if (!fd)
				open();
			
			Uint64 off = c->getIndex() * tor.getChunkSize();
			Uint8* buf = (Uint8*)fd->map(c,off,c->getSize(),CacheFile::RW);
			if (!buf)
			{
				mmap_failures++;
				// buffer it if mmapping fails
				Out(SYS_GEN|LOG_IMPORTANT) << "Warning : mmap failure, falling back to buffered mode" << endl;
				c->allocate();
				c->setStatus(Chunk::BUFFERED);
			}
			else
			{
				c->setData(buf,Chunk::MMAPPED);
			}
		}
		return true;
	}

	void SingleFileCache::load(Chunk* c)
	{
		if (!fd)
			open();
		
		Uint64 off = c->getIndex() * tor.getChunkSize();
		Uint8* buf = 0;
		if (mmap_failures >= 3 || !(buf = (Uint8*)fd->map(c,off,c->getSize(),CacheFile::READ)))
		{
			c->allocate();
			c->setStatus(Chunk::BUFFERED);
			fd->read(c->getData(),c->getSize(),off);
			if (mmap_failures < 3)
				mmap_failures++;
		}
		else
		{
			c->setData(buf,Chunk::MMAPPED);
		}
	}

	void SingleFileCache::save(Chunk* c)
	{
		if (!fd)
			open();
		
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
		if (!bt::Exists(output_file))
			bt::Touch(output_file);
		else
			preexisting_files = true;
	}
	
	void SingleFileCache::close()
	{
		if (fd)
		{
			fd->close();
			delete fd;
			fd = 0;
		}
	}
	
	void SingleFileCache::open()
	{	
		if (fd)
			return;
		
		try
		{
			fd = new CacheFile();
			fd->open(output_file,tor.getFileLength());
		}
		catch (...)
		{
			fd->close();
			delete fd;
			fd = 0;
			throw;
		}
	}
	
	void SingleFileCache::preallocateDiskSpace(PreallocationThread* prealloc)
	{
		if (!fd)
			open();
		
		if (!prealloc->isStopped())
			fd->preallocate(prealloc);
		else
			prealloc->setNotFinished();
	}
	
	bool SingleFileCache::hasMissingFiles(QStringList & sl)
	{
		if (!bt::Exists(output_file))
		{
			sl.append(output_file);
			return true;
		}
		return false;
	}

	void SingleFileCache::deleteDataFiles()
	{
		bt::Delete(output_file);
	}

	Uint64 SingleFileCache::diskUsage()
	{
		if (!fd)
			open();

		return fd->diskUsage();
	}
}
