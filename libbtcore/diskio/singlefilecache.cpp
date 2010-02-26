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
#include "singlefilecache.h"
#include <QTextStream>
#include <klocale.h>
#include <qfileinfo.h>
#include <qstringlist.h> 
#include <kio/copyjob.h>
#include <kio/jobuidelegate.h>
#include <util/fileops.h>
#include <util/error.h>
#include <util/functions.h>
#include <util/log.h>
#ifdef Q_WS_WIN
#include <util/win32.h>
#endif
#include <torrent/torrent.h>
#include "chunk.h"
#include "cachefile.h"
#include "piecedata.h"
#include "preallocationthread.h"
#include "deletedatafilesjob.h"
#include "movedatafilesjob.h"


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
	
	Job* SingleFileCache::moveDataFiles(const QString & ndir)
	{
		QString dst = ndir;
		if (!dst.endsWith(bt::DirSeparator()))
			dst += bt::DirSeparator();
		
		dst += output_file.mid(output_file.lastIndexOf(bt::DirSeparator()) + 1);
		if (output_file == dst)
			return 0;
		
		move_data_files_dst = dst;
		MoveDataFilesJob* job = new MoveDataFilesJob();
		job->addMove(output_file,dst);
		return job;
	}
	
	void SingleFileCache::moveDataFilesFinished(Job* job)
	{
		if (job->error() == KIO::ERR_USER_CANCELED)
		{
			if (bt::Exists(move_data_files_dst))
				bt::Delete(move_data_files_dst,true);
		}
		move_data_files_dst = QString();
	}
	
	PieceDataPtr SingleFileCache::createPiece(Chunk* c,Uint64 off,Uint32 length,bool read_only)
	{
		if (!fd)
			open();
			
		Uint64 piece_off = c->getIndex() * tor.getChunkSize() + off;
		Uint8* buf = 0;
		if (mmap_failures >= 3)
		{	
			buf = new Uint8[length];
			PieceData* cp = new PieceData(c,off,length,buf,0);
			insertPiece(c,cp);
			return PieceDataPtr(cp);
		}
		else
		{
			PieceData* cp = new PieceData(c,off,length,0,fd);
			buf = (Uint8*)fd->map(cp,piece_off,length,read_only ? CacheFile::READ : CacheFile::RW);
			if (buf)
			{
				cp->setData(buf);
			}
			else
			{
				if (mmap_failures < 3)
					mmap_failures++;
				
				delete cp;
				buf = new Uint8[length];
				cp = new PieceData(c,off,length,buf,0);
			}
			insertPiece(c,cp);
			return PieceDataPtr(cp);
		}
	}
	
	PieceDataPtr SingleFileCache::loadPiece(Chunk* c,Uint32 off,Uint32 length)
	{
		PieceDataPtr cp = findPiece(c,off,length);
		if (cp)
			return cp;
		
		cp = createPiece(c,off,length,true);
		if (cp && !cp->mapped())
		{
			// read data from file if piece isn't mapped
			Uint64 piece_off = c->getIndex() * tor.getChunkSize() + off;
			fd->read(cp->data(),length,piece_off);
		}
		
		return cp;
	}
	
	PieceDataPtr SingleFileCache::preparePiece(Chunk* c,Uint32 off,Uint32 length)
	{
		PieceDataPtr cp = findPiece(c,off,length);
		if (cp)
			return cp;
		
		return createPiece(c,off,length,false);
	}
	
	void SingleFileCache::savePiece(PieceDataPtr piece)
	{
		if (!fd)
			open();

		// mapped pieces will be unmapped when they are destroyed, buffered ones need to be written
		if (!piece->mapped())
		{
			Uint64 off = piece->parentChunk()->getIndex() * tor.getChunkSize() + piece->offset();
			if (piece->data())
				fd->write(piece->data(),piece->length(),off);
		}
	}

	void SingleFileCache::create()
	{
		// check for a to long path name
		if (FileNameToLong(output_file))
			output_file = ShortenFileName(output_file);
			
		if (!bt::Exists(output_file))
			bt::Touch(output_file);
		else
			preexisting_files = true;
		saveFileMap();
	}
	
	void SingleFileCache::close()
	{
		clearPieceCache();
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
			fd->open(output_file,tor.getTotalSize());
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

	Job* SingleFileCache::deleteDataFiles()
	{
		DeleteDataFilesJob* job = new DeleteDataFilesJob("");
		job->addFile(output_file);
		return job;
	}

	Uint64 SingleFileCache::diskUsage()
	{
		if (!fd)
			open();

		return fd->diskUsage();
	}
}
