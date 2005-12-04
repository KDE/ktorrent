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
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <kfileitem.h>
#include <kio/netaccess.h>
#include "mmapfile.h"

namespace bt
{

	MMapFile::MMapFile() : fd(-1),data(0),size(0),ptr(0),mode(READ)
	{}


	MMapFile::~MMapFile()
	{
		if (fd > 0)
			close();
	}

	bool MMapFile::open(const QString & file,Mode mode)
	{
		// close allready open file
		if (fd > 0)
			close();
		
		filename = file;
		
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
				flag = O_RDWR;
				mmap_flag = PROT_READ|PROT_WRITE;
				break;
		}
		
		// open the file
		fd = ::open(file.local8Bit() , O_RDONLY);//(int)flag);
		if (fd == -1)
			return false;
		
		KIO::UDSEntry entry;
		if (!KIO::NetAccess::stat(file,entry,0))
			return false;
		
		// read the file size
		size = KFileItem(entry,file).size();
		
		// mmap the file
		data = (Uint8*)mmap((caddr_t)0, size, PROT_READ, MAP_SHARED, fd, 0);
		if ((caddr_t)data == (caddr_t)(-1)) 
			return false;
		ptr = 0;
		return true;
	}
		
	void MMapFile::close()
	{
		if (fd > 0)
		{
			::close(fd);
			ptr = size = 0;
			data = 0;
			fd = -1;
			filename = QString::null;
		}
	}
		
	void MMapFile::flush()
	{
		// TODO: find flush function
		if (fd > 0) ;
			//::flush(fd);
	}
		
	Uint32 MMapFile::write(const void* buf,Uint32 buf_size)
	{
		if (fd == -1 || mode == READ)
			return 0;
		// memcpy data
		memcpy(&data[ptr],buf,buf_size);
		// update ptr
		ptr += buf_size;
		// update file size if necessary
		if (ptr >= size)
			size = ptr;
		
		return buf_size;
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
}

