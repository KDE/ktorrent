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

	CacheFile::CacheFile() : fd(-1),max_size(0),file_size(0)
	{}


	CacheFile::~CacheFile()
	{
		if (fd != -1)
			close(false);
	}
	
	void CacheFile::openFile()
	{
		fd = ::open(QFile::encodeName(path),O_RDWR | O_LARGEFILE);
		
		if (fd < 0)
		{
			throw Error(i18n("Cannot open %1 : %2").arg(path).arg(strerror(errno)));
		}

		file_size = FileSize(fd);

	//	Out() << QString("CacheFile %1 = %2").arg(path).arg(file_size) << endl;
		
		// re do all mappings if there are any
		QMap<void*,Entry>::iterator i = mappings.begin();
		while (i != mappings.end())
		{
			CacheFile::Entry e = i.data();
			i++;
			mappings.erase(e.ptr);
			e.ptr = map(e.thing,e.offset,e.size - e.diff,e.mode);
			if (e.ptr)
				e.thing->remapped(e.ptr);
		}
	}
	
	void CacheFile::open(const QString & path,Uint64 size)
	{
		// only set the path and the max size, we only open the file when it is needed
		this->path = path;
		max_size = size;
		// if there are mappings we must reopen the file and restore them
		if (mappings.count() > 0)
			openFile();
	}
		
	void* CacheFile::map(MMappeable* thing,Uint64 off,Uint32 size,Mode mode)
	{
		// reopen the file if necessary
		if (fd == -1)
		{
		//	Out() << "Reopening " << path << endl;
			openFile();
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
			Uint32 noff = off - diff;
		//	Out() << "Offsetted mmap : " << diff << endl;
			char* ptr = (char*)mmap(0, size + diff, mmap_flag, MAP_SHARED, fd, noff);
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
			void* ptr = mmap(0, size, mmap_flag, MAP_SHARED, fd, off);
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
			openFile();
		}
		
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
			if (to_write < 1024)
			{
				::write(fd,buf,to_write);
				to_write = 0;
			}
			else
			{
				::write(fd,buf,1024);
				to_write -= 1024;
			}
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
		// see if it wasn't an offsetted mapping
		if (mappings.contains(ptr))
		{
			CacheFile::Entry & e = mappings[ptr];
			if (e.diff > 0)
				munmap((char*)ptr - e.diff,e.size);
			else
				munmap(ptr,e.size);
			
			mappings.erase(ptr);
			// no mappings, close temporary
			if (mappings.count() == 0)
				closeTemporary();
		}
		else
		{
			munmap(ptr,size);
		}
	}
		
	void CacheFile::close(bool to_be_reopened)
	{
		if (fd == -1)
			return;
		
		QMap<void*,Entry>::iterator i = mappings.begin();
		while (i != mappings.end())
		{
			CacheFile::Entry & e = i.data();
			if (e.diff > 0)
				munmap((char*)e.ptr - e.diff,e.size);
			else
				munmap(e.ptr,e.size);
			e.thing->unmapped(to_be_reopened);
			// if it will be reopenend, we will not remove all mappings
			// so that they will be redone on reopening
			if (to_be_reopened)
			{
				i++;
			}
			else
			{
				i++;
				mappings.erase(e.ptr);
			}
		}
		::close(fd);
		fd = -1;
	}
	
	void CacheFile::read(Uint8* buf,Uint32 size,Uint64 off)
	{
		// reopen the file if necessary
		if (fd == -1)
		{
		//	Out() << "Reopening " << path << endl;
			openFile();
		}
		
		if (off >= file_size || off >= max_size)
		{
			throw Error(i18n("Error : Reading past the end of the file %1").arg(path));
		}
		
		// jump to right position
		SeekFile(fd,off,SEEK_SET);
		if ((Uint32)::read(fd,buf,size) != size)
			throw Error(i18n("Error reading from %1").arg(path));
	}
	
	void CacheFile::write(const Uint8* buf,Uint32 size,Uint64 off)
	{
		// reopen the file if necessary
		if (fd == -1)
		{
		//	Out() << "Reopening " << path << endl;
			openFile();
		}
		
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
		SeekFile(fd,off,SEEK_SET);
		int ret = ::write(fd,buf,size);
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
			
		close(fd);
		fd = -1;
		//Out() << "Temporarely closed " << path << endl;
	}
	
	
		
	void CacheFile::preallocate()
	{
		Out(SYS_GEN|LOG_NOTICE) << "Preallocating file " << path << " (" << max_size << " bytes)" << endl;
		bool close_again = false;
		if (fd == -1)
		{
			openFile();
			close_again = true;
		}

		try
		{
			bt::TruncateFile(fd,max_size);
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

	
}
