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
#include "multifilecache.h"
#include <errno.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include <QTextStream>
#include <klocale.h>
#include <util/file.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <util/error.h>
#include <util/log.h>
#include <torrent/torrent.h>
#include "cache.h"
#include "chunk.h"
#include "cachefile.h"
#include "dndfile.h"
#include "preallocationthread.h"
#include "movedatafilesjob.h"
#include <kdebug.h>


namespace bt
{
	static Uint64 FileOffset(Chunk* c,const TorrentFile & f,Uint64 chunk_size);
	static Uint64 FileOffset(Uint32 cindex,const TorrentFile & f,Uint64 chunk_size);
	static void DeleteEmptyDirs(const QString & output_dir,const QString & fpath);
	
	static void MakeFilePath(const QString & file)
	{
		QStringList sl = file.split(bt::DirSeparator());
		QString ctmp = bt::DirSeparator();
		
		for (Uint32 i = 0;i < sl.count() - 1;i++)
		{
			ctmp += sl[i];
			if (!bt::Exists(ctmp))
				MakeDir(ctmp);
			
			ctmp += bt::DirSeparator();
		}
	}
	

	MultiFileCache::MultiFileCache(Torrent& tor,const QString & tmpdir,const QString & datadir,bool custom_output_name) : Cache(tor, tmpdir,datadir)
	{
		cache_dir = tmpdir + "cache" + bt::DirSeparator();
		
		if (!custom_output_name)
			output_dir = this->datadir + tor.getNameSuggestion() + bt::DirSeparator();
		else
			output_dir = this->datadir;
		files.setAutoDelete(true);
	}


	MultiFileCache::~MultiFileCache()
	{}
	
	void MultiFileCache::loadFileMap()
	{
		QString file_map = tmpdir + "file_map";
		if (!bt::Exists(file_map))
		{
			QFile fptr(file_map);
			if (!fptr.open(QIODevice::WriteOnly))
				throw Error(i18n("Failed to create %1 : %2",file_map,fptr.errorString()));
			
			QTextStream out(&fptr);
			// file map doesn't exist, so create it based upon the output_dir
			Uint32 num = tor.getNumFiles();
			for (Uint32 i = 0;i < num;i++)
			{
				TorrentFile & tf = tor.getFile(i);
				tf.setPathOnDisk(output_dir + tf.getPath());
				out << tf.getPathOnDisk() << ::endl;
			}
		}
		else
		{
			QFile fptr(tmpdir + "file_map");
			if (!fptr.open(QIODevice::ReadOnly))
				throw Error(i18n("Failed to open %1 : %2",file_map,fptr.errorString()));
			
			Uint32 idx = 0;
			while (!fptr.atEnd() && idx < tor.getNumFiles())
			{
				QString path = QString::fromLocal8Bit(fptr.readLine().trimmed()); 
				tor.getFile(idx).setPathOnDisk(path);
				idx++;
			}
		}
	}
	
	void MultiFileCache::saveFileMap()
	{
		QString file_map = tmpdir + "file_map";
		QFile fptr(file_map);
		if (!fptr.open(QIODevice::WriteOnly))
			throw Error(i18n("Failed to create %1 : %2",file_map,fptr.errorString()));
			
		QTextStream out(&fptr);
		// file map doesn't exist, so create it based upon the output_dir
		Uint32 num = tor.getNumFiles();
		for (Uint32 i = 0;i < num;i++)
		{
			TorrentFile & tf = tor.getFile(i);
			out << tf.getPathOnDisk() << ::endl;
		}
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
		QString dnd_dir = tmpdir + "dnd" + bt::DirSeparator();
		// open all files
		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			TorrentFile & tf = tor.getFile(i);
			CacheFile* fd = 0;
			DNDFile* dfd = 0;
			try
			{
				if (!tf.doNotDownload())
				{
					if (files.contains(i))
						files.erase(i);
					
					fd = new CacheFile();
					fd->open(tf.getPathOnDisk(),tf.getSize());
					files.insert(i,fd);
				}
				else
				{
					if (dnd_files.contains(i))
						dnd_files.erase(i);
					
					dfd = new DNDFile(dnd_dir + tf.getPath() + ".dnd");
					dfd->checkIntegrity();
					dnd_files.insert(i,dfd);
				}
			}
			catch (...)
			{
				delete fd;
				fd = 0;
				delete dfd;
				dfd = 0;
				throw;
			}
		}
	}

	void MultiFileCache::changeTmpDir(const QString& ndir)
	{
		Cache::changeTmpDir(ndir);
		QString dnd_dir = tmpdir + "dnd" + bt::DirSeparator();
		
		// change paths for individual files, it should not
		// be a problem to move these files when they are open
		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			TorrentFile & tf = tor.getFile(i);
			if (tf.doNotDownload())
			{
				DNDFile* dfd = dnd_files.find(i);
				if (dfd)
					dfd->changePath(dnd_dir + tf.getPath() + ".dnd");
			}
		}
	}
	
	void MultiFileCache::changeOutputPath(const QString & outputpath)
	{
		output_dir = outputpath;
		if (!output_dir.endsWith(bt::DirSeparator()))
			output_dir += bt::DirSeparator();
		
		datadir = output_dir;
		
		Uint32 num = tor.getNumFiles();
		for (Uint32 i = 0;i < num;i++)
		{
			TorrentFile & tf = tor.getFile(i);
			tf.setPathOnDisk(output_dir + tf.getPath());
			CacheFile* cf = files.find(tf.getIndex());
			if (cf)
				cf->changePath(tf.getPathOnDisk());
		}
		saveFileMap();
	}
	
	KJob* MultiFileCache::moveDataFiles(const QString & ndir)
	{
		if ( !bt::Exists ( ndir ) )
			bt::MakeDir ( ndir );

		QString nd = ndir;
		if ( !nd.endsWith ( bt::DirSeparator() ) )
			nd += bt::DirSeparator();

		new_output_dir = nd;
		
		MoveDataFilesJob* job = new MoveDataFilesJob();
		
		for ( Uint32 i = 0;i < tor.getNumFiles();i++ )
		{
			TorrentFile & tf = tor.getFile ( i );
			if ( tf.doNotDownload() )
				continue;

				// check if every directory along the path exists, and if it doesn't
				// create it
			MakeFilePath(nd + tf.getPath());
				
			job->addMove(tf.getPathOnDisk(),nd + tf.getPath());
		}

		job->startMoving();
		return job;
	}
	
	void MultiFileCache::moveDataFilesFinished(KJob* job)
	{
		if (job->error())
			return;
		
		for ( Uint32 i = 0;i < tor.getNumFiles();i++ )
		{
			TorrentFile & tf = tor.getFile(i);
			tf.setPathOnDisk(new_output_dir + tf.getPath());
			CacheFile* cf = files.find(tf.getIndex());
			if (cf)
				cf->changePath(tf.getPathOnDisk());
			// check for empty directories and delete them
			DeleteEmptyDirs(output_dir,tf.getPath());
		}
	}
	
	KJob* MultiFileCache::moveDataFiles(const QMap<TorrentFileInterface*,QString> & files)
	{
		if (files.count() == 0)
			return 0;
		
		MoveDataFilesJob* job = new MoveDataFilesJob();
		QMap<TorrentFileInterface*,QString>::const_iterator i = files.begin();
		while (i != files.end())
		{
			TorrentFileInterface* tf = i.key();
			QString dest = i.value();
			if (QFileInfo(dest).isDir())
			{
				QString path = tf->getPath();
				if (!dest.endsWith(bt::DirSeparator()))
					dest += bt::DirSeparator();
			
				int last = path.lastIndexOf(bt::DirSeparator());
				job->addMove(tf->getPathOnDisk(),dest + path.mid(last+1));
			}
			else
				job->addMove(tf->getPathOnDisk(),i.value());
			i++;
		}
		
		job->startMoving();
		return job;
	}
		
	void MultiFileCache::moveDataFilesFinished(const QMap<TorrentFileInterface*,QString> & fmap,KJob* job)
	{
		if (job->error())
			return;
		
		QMap<TorrentFileInterface*,QString>::const_iterator i = fmap.begin();
		while (i != fmap.end())
		{
			TorrentFileInterface* tf = i.key();
			QString path = tf->getPathOnDisk();
			QString dest = i.value();
			if (QFileInfo(dest).isDir())
			{
				QString path = tf->getPath();
				if (!dest.endsWith(bt::DirSeparator()))
					dest += bt::DirSeparator();
			
				int last = path.lastIndexOf(bt::DirSeparator());
				tf->setPathOnDisk(dest + path.mid(last+1));
			}
			else
				tf->setPathOnDisk(i.value());
			
			CacheFile* cf = files.find(tf->getIndex());
			if (cf)
				cf->changePath(tf->getPathOnDisk());
			i++;
		}
		saveFileMap();
	}

	void MultiFileCache::create()
	{
		if (!bt::Exists(output_dir))
			MakeDir(output_dir);
		if (!bt::Exists(tmpdir + "dnd"))
			bt::MakeDir(tmpdir + "dnd");

		// update symlinks
		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{			
			TorrentFile & tf = tor.getFile(i);
			touch(tf);
		}
	}
	
	

	void MultiFileCache::touch(TorrentFile & tf)
	{
		QString fpath = tf.getPath();
		bool dnd = tf.doNotDownload();
		// first split fpath by / separator 
		QStringList sl = fpath.split(bt::DirSeparator());
		QString dtmp = tmpdir + "dnd" + bt::DirSeparator();
		MakeFilePath(dtmp + fpath); // make DND dir
		
		// then make the file
		if (dnd)
		{
			bt::Touch(tmpdir + "dnd" + bt::DirSeparator() + fpath);
		}
		else
		{
			MakeFilePath(tf.getPathOnDisk());
			if (!bt::Exists(tf.getPathOnDisk()))
			{
				bt::Touch(tf.getPathOnDisk());
			}
			else
			{
				preexisting_files = true;
				tf.setPreExisting(true); // mark the file as preexisting
			}
		}
	}

	void MultiFileCache::load(Chunk* c)
	{
		QList<Uint32> tflist;
		tor.calcChunkPos(c->getIndex(),tflist);
		
		// one file is simple, just mmap it
		if (tflist.count() == 1)
		{
			const TorrentFile & f = tor.getFile(tflist.first());
			CacheFile* fd = files.find(tflist.first());
			if (!fd)
				return;
			
			if (Cache::mappedModeAllowed() && mmap_failures < 3)
			{
				Uint64 off = FileOffset(c,f,tor.getChunkSize());
				Uint8* buf = (Uint8*)fd->map(c,off,c->getSize(),CacheFile::READ);
				if (buf)
				{
					c->setData(buf,Chunk::MMAPPED);
					// only return when the mapping is OK
					// if mmap fails we will just load it buffered
					return;
				}
				else
					mmap_failures++;
			}
		}
		
		Uint8* data = new Uint8[c->getSize()];
		Uint64 read = 0; // number of bytes read
		for (int i = 0;i < tflist.count();i++)
		{
			const TorrentFile & f = tor.getFile(tflist[i]);
			CacheFile* fd = files.find(tflist[i]);
			DNDFile* dfd = dnd_files.find(tflist[i]);
				
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
			if (fd)
				fd->read(data + read,to_read,off);
			else if (dfd)
			{
				Uint32 ret = 0;
				if (i == 0)
					ret = dfd->readLastChunk(data,read,c->getSize());
				else if (i == tflist.count() - 1)
					ret = dfd->readFirstChunk(data,read,c->getSize());
				else
					ret = dfd->readFirstChunk(data,read,c->getSize());
				
				if (ret > 0 && ret != to_read)
					Out(SYS_DIO|LOG_DEBUG) << "Warning : MultiFileCache::load ret != to_read" << endl;
			}
			read += to_read;
		}
		c->setData(data,Chunk::BUFFERED);
	}

	
	bool MultiFileCache::prep(Chunk* c)
	{
		// find out in which files a chunk lies
		QList<Uint32> tflist;
		tor.calcChunkPos(c->getIndex(),tflist);
		
//		Out(SYS_DIO|LOG_DEBUG) << "Prep " << c->getIndex() << endl;
		if (tflist.count() == 1)
		{
			// in one so just mmap it
			Uint64 off = FileOffset(c,tor.getFile(tflist.first()),tor.getChunkSize());
			CacheFile* fd = files.find(tflist.first());
			Uint8* buf = 0;
			if (fd && Cache::mappedModeAllowed() && mmap_failures < 3)
			{
				buf = (Uint8*)fd->map(c,off,c->getSize(),CacheFile::RW);
				if (!buf)
					mmap_failures++;
			}
			
			if (!buf)
			{
				// if mmap fails or is not possible use buffered mode
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
		QList<Uint32> tflist;
		tor.calcChunkPos(c->getIndex(),tflist);
		
		if (c->getStatus() == Chunk::MMAPPED)
		{
			// mapped chunks are easy
			CacheFile* fd = files.find(tflist[0]);
			if (!fd)
				return;
			
			fd->unmap(c->getData(),c->getSize());
			c->clear();
			c->setStatus(Chunk::ON_DISK);
			return;
		}
	
	//	Out(SYS_DIO|LOG_DEBUG) << "Writing to " << tflist.count() << " files " << endl;
		Uint64 written = 0; // number of bytes written
		for (int i = 0;i < tflist.count();i++)
		{
			const TorrentFile & f = tor.getFile(tflist[i]);
			CacheFile* fd = files.find(tflist[i]);
			DNDFile* dfd = dnd_files.find(tflist[i]);

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
			
		//	Out(SYS_DIO|LOG_DEBUG) << "to_write " << to_write << endl;
			// write the data
			if (fd)
				fd->write(c->getData() + written,to_write,off);
			else if (dfd)
			{
				if (i == 0)
					dfd->writeLastChunk(c->getData() + written,to_write);
				else if (i == tflist.count() - 1)
					dfd->writeFirstChunk(c->getData() + written,to_write);
				else
					dfd->writeFirstChunk(c->getData() + written,to_write);
			}
			
			written += to_write;
		}
		
		// set the chunk to on disk and clear it
		c->clear();
		c->setStatus(Chunk::ON_DISK);
	}
	
	void MultiFileCache::downloadStatusChanged(TorrentFile* tf, bool download)
	{	
		bool dnd = !download;
		QString dnd_dir = tmpdir + "dnd" + bt::DirSeparator();
		// if it is dnd and it is already in the dnd tree do nothing
		if (dnd && bt::Exists(dnd_dir + tf->getPath() + ".dnd"))
			return;
		
		// if it is !dnd and it is already in the output_dir tree do nothing
		if (!dnd && bt::Exists(tf->getPathOnDisk()))
			return;
		
		
		DNDFile* dfd = 0;
		CacheFile* fd = 0;
		try
		{
			
			if (dnd && bt::Exists(dnd_dir + tf->getPath()))
			{
				// old download, we need to convert it
				// save first and last chunk of the file
				saveFirstAndLastChunk(tf,dnd_dir + tf->getPath(),dnd_dir + tf->getPath() + ".dnd");
				// delete symlink
				bt::Delete(cache_dir + tf->getPath(),true);
				bt::Delete(dnd_dir + tf->getPath()); // delete old dnd file
				
				files.erase(tf->getIndex());
				dfd = new DNDFile(dnd_dir + tf->getPath() + ".dnd");
				dfd->checkIntegrity();
				dnd_files.insert(tf->getIndex(),dfd);
			}
			else if (dnd)
			{
				QString dnd_file = dnd_dir + tf->getPath() + ".dnd";
				// save first and last chunk of the file
				if (bt::Exists(tf->getPathOnDisk()))
					saveFirstAndLastChunk(tf,tf->getPathOnDisk(),dnd_file);
				
				// delete data file
				bt::Delete(tf->getPathOnDisk(),true);
				
				files.erase(tf->getIndex());
				dfd = new DNDFile(dnd_file);
				dfd->checkIntegrity();
				dnd_files.insert(tf->getIndex(),dfd);
			}
			else
			{
				// recreate the file
				recreateFile(tf,dnd_dir + tf->getPath() + ".dnd",tf->getPathOnDisk());
				bt::Delete(dnd_dir + tf->getPath() + ".dnd");
				dnd_files.erase(tf->getIndex());
				fd = new CacheFile();
				fd->open(tf->getPathOnDisk(),tf->getSize());
				files.insert(tf->getIndex(),fd);
			}
		}
		catch (bt::Error & err)
		{
			delete fd;
			delete dfd;
			Out(SYS_DIO|LOG_DEBUG) << err.toString() << endl;
		}
	}
	

	
	void MultiFileCache::saveFirstAndLastChunk(TorrentFile* tf,const QString & src_file,const QString & dst_file)
	{
		DNDFile out(dst_file);
		File fptr;
		if (!fptr.open(src_file,"rb"))
			throw Error(i18n("Cannot open file %1 : %2",src_file,fptr.errorString()));
		
		Uint32 cs = 0;
		if (tf->getFirstChunk() == tor.getNumChunks() - 1)
		{
			cs = tor.getFileLength() % tor.getChunkSize();
			if (cs == 0)
				cs = tor.getChunkSize();
		}
		else
			cs = tor.getChunkSize();
		
		Uint8* tmp = new Uint8[tor.getChunkSize()];
		try
		{
			fptr.read(tmp,cs - tf->getFirstChunkOffset());
			out.writeFirstChunk(tmp,cs - tf->getFirstChunkOffset());
			
			if (tf->getFirstChunk() != tf->getLastChunk())
			{
				Uint64 off = FileOffset(tf->getLastChunk(),*tf,tor.getChunkSize());
				fptr.seek(File::BEGIN,off);
				fptr.read(tmp,tf->getLastChunkSize());
				out.writeLastChunk(tmp,tf->getLastChunkSize());
			}
			delete [] tmp;
		}
		catch (...)
		{
			delete [] tmp;
			throw;
		}
	}
	
	void MultiFileCache::recreateFile(TorrentFile* tf,const QString & dnd_file,const QString & output_file)
	{
		DNDFile dnd(dnd_file);
		
		// make sure path exists
		MakeFilePath(output_file);
		// create the output file
		bt::Touch(output_file);
		// truncate it
		try
		{
			bool res = false;
			
			#ifdef HAVE_XFS_XFS_H
				if( (! res) && Cache::useFSSpecificPreallocMethod() )
				{
					res = XfsPreallocate(output_file, tf->getSize()) );
				}
			#endif
			
			if(! res)
			{
				bt::TruncateFile(output_file,tf->getSize());
			}
		}
		catch (bt::Error & e)
		{
			// first attempt failed, must be fat so try that
			if (!FatPreallocate(output_file,tf->getSize()))
			{	
				throw Error(i18n("Cannot preallocate diskspace : %1",strerror(errno)));
			}
		}
		
		Uint32 cs = 0;
		if (tf->getFirstChunk() == tor.getNumChunks() - 1)
		{
			cs = tor.getFileLength() % tor.getChunkSize();
			if (cs == 0)
				cs = tor.getChunkSize();
		}
		else
			cs = tor.getChunkSize();
		
		File fptr;
		if (!fptr.open(output_file,"r+b"))
			throw Error(i18n("Cannot open file %1 : %2",output_file,fptr.errorString()));
			
		
		Uint32 ts = cs - tf->getFirstChunkOffset() > tf->getLastChunkSize() ? 
				cs - tf->getFirstChunkOffset() : tf->getLastChunkSize();
		Uint8* tmp = new Uint8[ts];
		
		try
		{
			dnd.readFirstChunk(tmp,0,cs - tf->getFirstChunkOffset());
			fptr.write(tmp,cs - tf->getFirstChunkOffset());
			
			if (tf->getFirstChunk() != tf->getLastChunk())
			{
				Uint64 off = FileOffset(tf->getLastChunk(),*tf,tor.getChunkSize());
				fptr.seek(File::BEGIN,off);
				dnd.readLastChunk(tmp,0,tf->getLastChunkSize());
				fptr.write(tmp,tf->getLastChunkSize());
			}
			delete [] tmp;
		}
		catch (...)
		{
			delete [] tmp;
			throw;
		}
	}
	
	void MultiFileCache::preallocateDiskSpace(PreallocationThread* prealloc)
	{
		Out(SYS_DIO|LOG_DEBUG) << "MultiFileCache::preallocateDiskSpace" << endl;
		PtrMap<Uint32,CacheFile>::iterator i = files.begin();
		while (i != files.end())
		{
			CacheFile* cf = i->second;
			if (!prealloc->isStopped())
			{
				cf->preallocate(prealloc);
			}
			else
			{
				// we got interrupted tell the thread we are not finished and return
				prealloc->setNotFinished();
				return;
			}
			i++;
		}
	}
	
	bool MultiFileCache::hasMissingFiles(QStringList & sl)
	{
		bool ret = false;
		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			TorrentFile & tf = tor.getFile(i);
			if (tf.doNotDownload())
				continue;
			
			QString p = tf.getPathOnDisk();
			if (!bt::Exists(p))
			{
				ret = true;
				tf.setMissing(true);
				sl.append(p);
			}
			else
				tf.setMissing(false);
		}
		return ret;
	}
	
	static void DeleteEmptyDirs(const QString & output_dir,const QString & fpath)
	{
		QStringList sl = fpath.split(bt::DirSeparator());
		// remove the last, which is just the filename
		sl.pop_back();
		
		while (sl.count() > 0)
		{
			QString path = output_dir;
			// reassemble the full directory path
			for (QStringList::iterator itr = sl.begin(); itr != sl.end();itr++)
				path += *itr + bt::DirSeparator();
			
			QDir dir(path);
			QStringList el = dir.entryList(QDir::AllEntries|QDir::System|QDir::Hidden);
			el.removeAll(".");
			el.removeAll("..");
			if (el.count() == 0)
			{
				// no childern so delete the directory
				Out(SYS_GEN|LOG_DEBUG) << "Deleting empty directory : " << path << endl;
				bt::Delete(path,true);
				sl.pop_back(); // remove the last so we can go one higher
			}
			else
			{
				
				// children, so we cannot delete any more directories higher up
				return;
			}
		}
		
		// now the output_dir itself
		QDir dir(output_dir);
		QStringList el = dir.entryList(QDir::AllEntries|QDir::System|QDir::Hidden);
		el.removeAll(".");
		el.removeAll("..");
		if (el.count() == 0)
		{
			Out(SYS_GEN|LOG_DEBUG) << "Deleting empty directory : " << output_dir << endl;
			bt::Delete(output_dir,true);
		}
	}
	
	void MultiFileCache::deleteDataFiles()
	{
		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			TorrentFile & tf = tor.getFile(i);
			QString fpath = tf.getPathOnDisk();
			if (!tf.doNotDownload())
			{
				// first delete the file
				bt::Delete(fpath);
			}
			
			// check for subdirectories
			DeleteEmptyDirs(output_dir,tf.getPath());
		}
	}
	
	Uint64 MultiFileCache::diskUsage()
	{
		Uint64 sum = 0;

		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			TorrentFile & tf = tor.getFile(i);
			if (tf.doNotDownload())
				continue;

			try
			{
				CacheFile* cf = files.find(i);
				if (cf)
				{
					sum += cf->diskUsage();
				}
				else
				{
					// doesn't exist yet, must be before open is called
					// so create one and delete it right after
					cf = new CacheFile();
					cf->open(tf.getPathOnDisk(),tf.getSize());
					sum += cf->diskUsage();
					delete cf;
				}
			}
			catch (bt::Error & err) // make sure we catch any exceptions
			{
				Out(SYS_DIO|LOG_DEBUG) << "Error: " << err.toString() << endl;
			}
		}

		return sum;
	}
	
	
	///////////////////////////////

	Uint64 FileOffset(Chunk* c,const TorrentFile & f,Uint64 chunk_size)
	{
		return FileOffset(c->getIndex(),f,chunk_size);
	}
	
	Uint64 FileOffset(Uint32 cindex,const TorrentFile & f,Uint64 chunk_size)
	{
		return f.fileOffset(cindex,chunk_size);
	}
	
}
