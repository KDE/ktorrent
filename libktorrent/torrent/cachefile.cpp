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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <qfile.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kfileitem.h>
#include <util/array.h>
#include <util/fileops.h>
#include <torrent/globals.h>
#include <interfaces/functions.h>
#include <kapplication.h>
#include <util/log.h>
#include <util/error.h>
#include "cachefile.h"
#include "preallocationthread.h"
#include "settings.h"


// Not all systems have an O_LARGEFILE - Solaris depending
// on command-line defines, FreeBSD never - so in those cases,
// make it a zero bitmask. As long as it's only OR'ed into
// open(2) flags, that's fine.
//
#ifndef O_LARGEFILE
#define O_LARGEFILE (0)
#endif




namespace bt
{

	CacheFile::CacheFile() : fd(-1),max_size(0),file_size(0),mutex(true)
	{
		read_only = false;
	}


	CacheFile::~CacheFile()
	{
		if (fd != -1)
			close();
	}
	
	void CacheFile::changePath(const QString & npath)
	{
		path = npath;
	}
	
	void CacheFile::openFile(Mode mode)
	{
		int flags = O_LARGEFILE;
		
		// by default allways try read write
		fd = ::open(QFile::encodeName(path),flags | O_RDWR);
		if (fd < 0 && mode == READ)
		{
			// in case RDWR fails, try readonly if possible
			fd = ::open(QFile::encodeName(path),flags | O_RDONLY);
			if (fd >= 0)
				read_only = true;
		}
		
		if (fd < 0)
		{
			throw Error(i18n("Cannot open %1 : %2").arg(path).arg(strerror(errno)));
		}
		
		file_size = FileSize(fd);
	}
	
	void CacheFile::open(const QString & path,Uint64 size)
	{
		QMutexLocker lock(&mutex);
		// only set the path and the max size, we only open the file when it is needed
		this->path = path;
		max_size = size;
	}
		
	void* CacheFile::map(MMappeable* thing,Uint64 off,Uint32 size,Mode mode)
	{
		QMutexLocker lock(&mutex);
		// reopen the file if necessary
		if (fd == -1)
		{
		//	Out() << "Reopening " << path << endl;
			openFile(mode);
		}
		
		if (read_only && mode != READ)
		{
			throw Error(i18n("Cannot open %1 for writing : readonly filesystem").arg(path));
		}
		
		if (off + size > max_size)
		{
			Out() << "Warning : writing past the end of " << path << endl;
			Out() << (off + size) << " " << max_size << endl;
			return 0;
		}
		
		int mmap_flag = 0;
		switch (mode)
		{
			case READ:
				mmap_flag = PROT_READ;
				break;
			case WRITE:
				mmap_flag = PROT_WRITE;
				break;
			case RW:
				mmap_flag = PROT_READ|PROT_WRITE;
				break;
		}
		
		if (off + size > file_size)
		{
			Uint64 to_write = (off + size) - file_size;
		//	Out() << "Growing file with " << to_write << " bytes" << endl;
			growFile(to_write);
		}
		
		Uint32 page_size = sysconf(_SC_PAGESIZE);
		if (off % page_size > 0)
		{
			// off is not a multiple of the page_size
			// so we play around a bit
			Uint32 diff = (off % page_size);
			Uint64 noff = off - diff;
		//	Out() << "Offsetted mmap : " << diff << endl;
#if HAVE_MMAP64
			char* ptr = (char*)mmap64(0, size + diff, mmap_flag, MAP_SHARED, fd, noff);
#else
			char* ptr = (char*)mmap(0, size + diff, mmap_flag, MAP_SHARED, fd, noff);
#endif
			if (ptr == MAP_FAILED) 
			{
				Out() << "mmap failed : " << QString(strerror(errno)) << endl;
				return 0;
			}
			else
			{
				CacheFile::Entry e;
				e.thing = thing;
				e.offset = off;
				e.diff = diff;
				e.ptr = ptr;
				e.size = size + diff;
				e.mode = mode;
				mappings.insert((void*)(ptr + diff),e);
				return ptr + diff;
			}
		}
		else
		{
#if HAVE_MMAP64
			void* ptr = mmap64(0, size, mmap_flag, MAP_SHARED, fd, off);
#else
			void* ptr = mmap(0, size, mmap_flag, MAP_SHARED, fd, off);
#endif
			if (ptr == MAP_FAILED) 
			{
				Out() << "mmap failed : " << QString(strerror(errno)) << endl;
				return 0;
			}
			else
			{
				CacheFile::Entry e;
				e.thing = thing;
				e.offset = off;
				e.ptr = ptr;
				e.diff = 0;
				e.size = size;
				e.mode = mode;
				mappings.insert(ptr,e);
				return ptr;
			}
		}
	}
	
	void CacheFile::growFile(Uint64 to_write)
	{
		// reopen the file if necessary
		if (fd == -1)
		{
		//	Out() << "Reopening " << path << endl;
			openFile(RW);
		}
		
		if (read_only)
			throw Error(i18n("Cannot open %1 for writing : readonly filesystem").arg(path));
		
		// jump to the end of the file
		SeekFile(fd,0,SEEK_END);
		
		if (file_size + to_write > max_size)
		{
			Out() << "Warning : writing past the end of " << path << endl;
			Out() << (file_size + to_write) << " " << max_size << endl;
		}
		
		Uint8 buf[1024];
		memset(buf,0,1024);
		Uint64 num = to_write;
		// write data until to_write is 0
		while (to_write > 0)
		{
			int nb = to_write > 1024 ? 1024 : to_write;
			int ret = ::write(fd,buf,nb);
			if (ret < 0)
				throw Error(i18n("Cannot expand file %1 : %2").arg(path).arg(strerror(errno)));
			else if (ret != nb)
				throw Error(i18n("Cannot expand file %1 : incomplete write").arg(path));
			to_write -= nb;
		}
		file_size += num;
//		
	//	Out() << QString("growing %1 = %2").arg(path).arg(kt::BytesToString(file_size)) << endl;

		if (file_size != FileSize(fd))
		{
//			Out() << QString("Homer Simpson %1 %2").arg(file_size).arg(sb.st_size) << endl;
			fsync(fd);
			if (file_size != FileSize(fd))
			{
				throw Error(i18n("Cannot expand file %1").arg(path));
			}
		}
	}
		
	void CacheFile::unmap(void* ptr,Uint32 size)
	{
		int ret = 0;
		QMutexLocker lock(&mutex);
		// see if it wasn't an offsetted mapping
		if (mappings.contains(ptr))
		{
			CacheFile::Entry & e = mappings[ptr];
#if HAVE_MUNMAP64
			if (e.diff > 0)
				ret = munmap64((char*)ptr - e.diff,e.size);
			else
				ret = munmap64(ptr,e.size);
#else
			if (e.diff > 0)
				ret = munmap((char*)ptr - e.diff,e.size);
			else
				ret = munmap(ptr,e.size);
#endif
			mappings.erase(ptr);
			// no mappings, close temporary
			if (mappings.count() == 0)
				closeTemporary();
		}
		else
		{
#if HAVE_MUNMAP64
			ret = munmap64(ptr,size);
#else
			ret = munmap(ptr,size);
#endif
		}
		
		if (ret < 0)
		{
			Out(SYS_DIO|LOG_IMPORTANT) << QString("Munmap failed with error %1 : %2").arg(errno).arg(strerror(errno)) << endl;
		}
	}
		
	void CacheFile::close()
	{
		QMutexLocker lock(&mutex);
		
		if (fd == -1)
			return;
		
		QMap<void*,Entry>::iterator i = mappings.begin();
		while (i != mappings.end())
		{
			int ret = 0;
			CacheFile::Entry & e = i.data();
#if HAVE_MUNMAP64
			if (e.diff > 0)
				ret = munmap64((char*)e.ptr - e.diff,e.size);
			else
				ret = munmap64(e.ptr,e.size);
#else
			if (e.diff > 0)
				ret = munmap((char*)e.ptr - e.diff,e.size);
			else
				ret = munmap(e.ptr,e.size);
#endif
			e.thing->unmapped();
			
			i++;
			mappings.erase(e.ptr);
						
			if (ret < 0)
			{
				Out(SYS_DIO|LOG_IMPORTANT) << QString("Munmap failed with error %1 : %2").arg(errno).arg(strerror(errno)) << endl;
			}	
		}
		::close(fd);
		fd = -1;
	}
	
	void CacheFile::read(Uint8* buf,Uint32 size,Uint64 off)
	{
		QMutexLocker lock(&mutex);
		bool close_again = false;
		
		// reopen the file if necessary
		if (fd == -1)
		{
		//	Out() << "Reopening " << path << endl;
			openFile(READ);
			close_again = true;
		}
		
		if (off >= file_size || off >= max_size)
		{
			throw Error(i18n("Error : Reading past the end of the file %1").arg(path));
		}
		
		// jump to right position
		SeekFile(fd,(Int64)off,SEEK_SET);
		if ((Uint32)::read(fd,buf,size) != size)
		{
			if (close_again)
				closeTemporary();
			
			throw Error(i18n("Error reading from %1").arg(path));
		}
		
		if (close_again)
			closeTemporary();
	}
	
	void CacheFile::write(const Uint8* buf,Uint32 size,Uint64 off)
	{
		QMutexLocker lock(&mutex);
		bool close_again = false;
		
		// reopen the file if necessary
		if (fd == -1)
		{
		//	Out() << "Reopening " << path << endl;
			openFile(RW);
			close_again = true;
		}
		
		if (read_only)
			throw Error(i18n("Cannot open %1 for writing : readonly filesystem").arg(path));
		
		if (off + size > max_size)
		{
			Out() << "Warning : writing past the end of " << path << endl;
			Out() << (off + size) << " " << max_size << endl;
		}
		
		if (file_size < off)
		{
			//Out() << QString("Writing %1 bytes at %2").arg(size).arg(off) << endl;
			growFile(off - file_size);
		}
		
		// jump to right position
		SeekFile(fd,(Int64)off,SEEK_SET);
		int ret = ::write(fd,buf,size);
		if (close_again)
			closeTemporary();
		
		if (ret == -1)
			throw Error(i18n("Error writing to %1 : %2").arg(path).arg(strerror(errno)));
		else if ((Uint32)ret != size)
		{
			Out() << QString("Incomplete write of %1 bytes, should be %2").arg(ret).arg(size) << endl;
			throw Error(i18n("Error writing to %1").arg(path));
		}
		
		if (off + size > file_size)
			file_size = off + size;
	}
	
	void CacheFile::closeTemporary()
	{
		if (fd == -1 || mappings.count() > 0)
			return;
			
		::close(fd);
		fd = -1;
	}
	
	
		
	void CacheFile::preallocate(PreallocationThread* prealloc)
	{
		QMutexLocker lock(&mutex);
		
		if (FileSize(path) == max_size)
		{
			Out(SYS_GEN|LOG_NOTICE) << "File " << path << " already big enough" << endl;
			return;
		}
		
		Out(SYS_GEN|LOG_NOTICE) << "Preallocating file " << path << " (" << max_size << " bytes)" << endl;
		bool close_again = false;
		if (fd == -1)
		{
			openFile(RW);
			close_again = true;
		}
		
		if (read_only)
		{
			if (close_again)
				closeTemporary();
			
			throw Error(i18n("Cannot open %1 for writing : readonly filesystem").arg(path));
		}

		try
		{
			bool res = false;
			
			#ifdef HAVE_XFS_XFS_H
				if( (! res) && Settings::fullDiskPrealloc() && (Settings::fullDiskPreallocMethod() == 1) )
				{
					res = XfsPreallocate(fd, max_size);
				}
			#endif
			
			if(! res)
			{
				bt::TruncateFile(fd,max_size,!Settings::fullDiskPrealloc());
			}
		}
		catch (bt::Error & e)
		{
			// first attempt failed, must be fat so try that
			if (!FatPreallocate(fd,max_size))
			{
				if (close_again)
					closeTemporary();
				
				throw Error(i18n("Cannot preallocate diskspace : %1").arg(strerror(errno)));
			}
		}

		file_size = FileSize(fd);
		Out(SYS_GEN|LOG_DEBUG) << "file_size = " << file_size << endl;
		if (close_again)
			closeTemporary();
	}

	Uint64 CacheFile::diskUsage()
	{
		Uint64 ret = 0;
		bool close_again = false;
		if (fd == -1)
		{
			openFile(READ);
			close_again = true;
		}

		struct stat sb;
		if (fstat(fd,&sb) == 0)
		{
			ret = (Uint64)sb.st_blocks * 512;
		}
		
	//	Out(SYS_GEN|LOG_NOTICE) << "CF: " << path << " is taking up " << ret << " bytes" << endl;
		if (close_again)
			closeTemporary();

		return ret;
	}
}
