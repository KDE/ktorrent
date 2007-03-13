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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <qfile.h>
#include <kfileitem.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <util/error.h>
#include <util/log.h>
#include <torrent/globals.h>
#include "mmapfile.h"

namespace bt
{

	MMapFile::MMapFile() : fd(-1),data(0),size(0),file_size(0),ptr(0),mode(READ)
	{}


	MMapFile::~MMapFile()
	{
		if (fd > 0)
			close();
	}

	bool MMapFile::open(const QString & file,Mode mode)
	{
#if HAVE_STAT64
		struct stat64 sb;
		stat64(QFile::encodeName(file),&sb);
#else
		struct stat sb;
		stat(QFile::encodeName(file),&sb);
#endif
		
		return open(file,mode,(Uint64)sb.st_size);
	}
	
	bool MMapFile::open(const QString & file,Mode mode,Uint64 size)
	{
		// close already open file
		if (fd > 0)
			close();
		
		// setup flags
		int flag = 0,mmap_flag = 0;
		switch (mode)
		{
			case READ:
				flag = O_RDONLY;
				mmap_flag = PROT_READ;
				break;
			case WRITE:
				flag = O_WRONLY | O_CREAT;
				mmap_flag = PROT_WRITE;
				break;
			case RW:
				flag = O_RDWR | O_CREAT;
				mmap_flag = PROT_READ|PROT_WRITE;
				break;
		}

		// Not all systems have O_LARGEFILE as an explicit flag
		// (for instance, FreeBSD. Solaris does, but only if
		// _LARGEFILE_SOURCE is defined in the compile).
		// So OR it in if it is defined.
#ifdef O_LARGEFILE
		flag |= O_LARGEFILE;
#endif

		// open the file
		fd = ::open(QFile::encodeName(file) , flag);//(int)flag);
		if (fd == -1)
			return false;
		
		// read the file size
		this->size = size;
		this->mode = mode;
		
#if HAVE_STAT64
		struct stat64 sb;
		stat64(QFile::encodeName(file),&sb);
#else
		struct stat sb;
		stat(QFile::encodeName(file),&sb);
#endif
		file_size = (Uint64)sb.st_size;
		filename = file;
		
		// mmap the file
#if HAVE_MMAP64
		data = (Uint8*)mmap64(0, size, mmap_flag, MAP_SHARED, fd, 0);
#else
		data = (Uint8*)mmap(0, size, mmap_flag, MAP_SHARED, fd, 0);
#endif
		if (data == MAP_FAILED) 
		{
			::close(fd);
			data = 0;
			fd = -1;
			ptr = 0;
			return false;
		}
		ptr = 0;
		return true;
	}
		
	void MMapFile::close()
	{
		if (fd > 0)
		{
#if HAVE_MUNMAP64
			munmap64(data,size);
#else
			munmap(data,size);
#endif
			::close(fd);
			ptr = size = 0;
			data = 0;
			fd = -1;
			filename = QString::null;
		}
	}
		
	void MMapFile::flush()
	{
		if (fd > 0)
			msync(data,size,MS_SYNC);
	}
		
	Uint32 MMapFile::write(const void* buf,Uint32 buf_size)
	{
		if (fd == -1 || mode == READ)
			return 0;
		
		// check if data fits in memory mapping
		if (ptr + buf_size > size)
			throw Error(i18n("Cannot write beyond end of the mmap buffer!"));
		
		Out() << "MMapFile::write : " << (ptr + buf_size) << " " << file_size << endl;
		// enlarge the file if necessary
		if (ptr + buf_size > file_size)
		{
			growFile(ptr + buf_size);
		}
		
		// memcpy data
		memcpy(&data[ptr],buf,buf_size);
		// update ptr
		ptr += buf_size;
		// update file size if necessary
		if (ptr >= size)
			size = ptr;
		
		return buf_size;
	}
	
	void MMapFile::growFile(Uint64 new_size)
	{
		Out() << "Growing file to " << new_size << " bytes " << endl;
		Uint64 to_write = new_size - file_size;
		ssize_t written;
		// jump to the end of the file
		lseek(fd,0,SEEK_END);
		
		Uint8 buf[1024];
		memset(buf,0,1024);
		// write data until to_write is 0
		while (to_write > 0)
		{
			ssize_t w = ::write(fd,buf, to_write > 1024 ? 1024 : to_write);
			if (w > 0)
			    to_write -= w;
			else if (w < 0)
			    break;
		}
		file_size = new_size;
	}
		
	Uint32 MMapFile::read(void* buf,Uint32 buf_size)
	{
		if (fd == -1 || mode == WRITE)
			return 0;
		
		// check if we aren't going to read past the end of the file
		Uint32 to_read = ptr + buf_size >= size ? size - ptr : buf_size;
		// read data
		memcpy(buf,data+ptr,to_read);
		ptr += to_read;
		return to_read;
	}

	Uint64 MMapFile::seek(SeekPos from,Int64 num)
	{
		switch (from)
		{
			case BEGIN:
				if (num > 0)
					ptr = num; 
				if (ptr >= size)
					ptr = size - 1;
				break;
			case END:
				{
					Int64 np = (size - 1) + num;
					if (np < 0)
					{
						ptr = 0;
						break;
					}
					if (np >= (Int64) size)
					{
						ptr = size - 1;
						break;
					}
					ptr = np;
				}
				break;
			case CURRENT:
				{
					Int64 np = ptr + num;
					if (np < 0)
					{
						ptr = 0;
						break;
					}
					if (np >= (Int64) size)
					{
						ptr = size - 1;
						break;
					}
					ptr = np;
				}	
				break;
		}
		return ptr;
	}

	bool MMapFile::eof() const
	{
		return ptr >= size;
	}

	Uint64 MMapFile::tell() const
	{
		return ptr;
	}
	
	QString MMapFile::errorString() const
	{
		return strerror(errno);
	}
	
	Uint64 MMapFile::getSize() const
	{
		return size;
	}
	
	Uint8* MMapFile::getData(Uint64 off)
	{
		if (off >= size)
			return 0;
		return &data[off];
	}
}

