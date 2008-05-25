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
#include <klocale.h>
#include <util/file.h>
#include <util/error.h>
#include <util/fileops.h>
#include <util/sha1hash.h>
#include "dndfile.h"

namespace bt
{
	const Uint32 DND_FILE_HDR_MAGIC = 0xD1234567;
	
	struct DNDFileHeader
	{
		Uint32 magic;
		Uint32 first_size;
		Uint32 last_size;
		Uint8 data_sha1[20];
	};

	DNDFile::DNDFile(const QString & path) : path(path)
	{}


	DNDFile::~DNDFile()
	{}
	
	void DNDFile::changePath(const QString & npath)
	{
		path = npath;
	}
	
	void DNDFile::checkIntegrity()
	{
		File fptr;
		if (!fptr.open(path,"rb"))
		{
			create();
			return;
		}
		
		DNDFileHeader hdr;
		if (fptr.read(&hdr,sizeof(DNDFileHeader)) != sizeof(DNDFileHeader))
		{
			create();
			return;
		}
		
		if (hdr.magic != DND_FILE_HDR_MAGIC && bt::FileSize(path) != sizeof(DNDFileHeader) + hdr.first_size + hdr.last_size)
		{
			create();
			return;
		}
		
#if 0
		if (hdr.first_size > 0 || hdr.last_size > 0)
		{
			// check hash
			Uint32 data_size = hdr.first_size + hdr.last_size;
			Uint8* buf = new Uint8[data_size];
			if (fptr.read(buf,data_size) != data_size)
			{
				delete [] buf;
				create();
				return;
			}
			
			if (SHA1Hash::generate(buf,data_size) != SHA1Hash(hdr.data_sha1))
			{
				delete [] buf;
				create();
				return;
			}
			
			delete [] buf;
		}
#endif
	}
	
	void DNDFile::create()
	{
		DNDFileHeader hdr;
		hdr.magic = DND_FILE_HDR_MAGIC;
		hdr.first_size = 0;
		hdr.last_size = 0;
		memset(hdr.data_sha1,0,20);
		
		File fptr;
		if (!fptr.open(path,"wb"))
			throw Error(i18n("Cannot create file %1 : %2").arg(path).arg(fptr.errorString()));
		
		fptr.write(&hdr,sizeof(DNDFileHeader));
		fptr.close();
	}
	
	
	
	Uint32 DNDFile::readFirstChunk(Uint8* buf,Uint32 off,Uint32 buf_size)
	{
		File fptr;
		if (!fptr.open(path,"rb"))
		{
			create();
			return 0;
		}
		
		DNDFileHeader hdr;
		if (fptr.read(&hdr,sizeof(DNDFileHeader)) != sizeof(DNDFileHeader))
		{
			create();
			return 0;
		}
		
		if (hdr.first_size == 0)
			return 0;
		
		if (hdr.first_size + off > buf_size)
			return 0;
		
		return fptr.read(buf + off,hdr.first_size);
	}
	
	Uint32 DNDFile::readLastChunk(Uint8* buf,Uint32 off,Uint32 buf_size)
	{	
		File fptr;
		if (!fptr.open(path,"rb"))
		{
			create();
			return 0;
		}
		
		DNDFileHeader hdr;
		if (fptr.read(&hdr,sizeof(DNDFileHeader)) != sizeof(DNDFileHeader))
		{
			create();
			return 0;
		}
		
		if (hdr.last_size == 0)
			return 0;
		
		if (hdr.last_size + off > buf_size)
			return 0;
		
		fptr.seek(File::BEGIN,sizeof(DNDFileHeader) + hdr.first_size);
		return fptr.read(buf + off,hdr.last_size);
	}

	void DNDFile::writeFirstChunk(const Uint8* buf,Uint32 fc_size)
	{
		File fptr;
		if (!fptr.open(path,"r+b"))
		{
			create();
			if (!fptr.open(path,"r+b"))
			{
				throw Error(i18n("Failed to write first chunk to DND file : %1").arg(fptr.errorString()));
			}
		}
		
		DNDFileHeader hdr;
		fptr.read(&hdr,sizeof(DNDFileHeader));
		if (hdr.last_size == 0)
		{
			hdr.first_size = fc_size;
			fptr.seek(File::BEGIN,0);
			// update hash first
		//	SHA1Hash h = SHA1Hash::generate(buf,fc_size);
		//	memcpy(hdr.data_sha1,h.getData(),20);
			// write header
			fptr.write(&hdr,sizeof(DNDFileHeader));
			// write data
			fptr.write(buf,fc_size);
		}
		else
		{
			hdr.first_size = fc_size;
			Uint8* tmp = new Uint8[hdr.first_size + hdr.last_size];
			try
			{
				
				// put everything in tmp buf
				memcpy(tmp,buf,hdr.first_size);
				fptr.seek(File::BEGIN,sizeof(DNDFileHeader) + hdr.first_size);
				fptr.read(tmp + hdr.first_size,hdr.last_size);
				
				// update the hash of the header
		//		SHA1Hash h = SHA1Hash::generate(tmp,hdr.first_size + hdr.last_size);
		//		memcpy(hdr.data_sha1,h.getData(),20);
				
				// write header + data
				fptr.seek(File::BEGIN,0);
				fptr.write(&hdr,sizeof(DNDFileHeader));
				fptr.write(tmp,hdr.first_size + hdr.last_size);
				delete [] tmp;
				
			}
			catch (...)
			{
				delete [] tmp;
				throw;
			}
		}
	}
		
	
	void DNDFile::writeLastChunk(const Uint8* buf,Uint32 lc_size)
	{
		File fptr;
		if (!fptr.open(path,"r+b"))
		{
			create();
			if (!fptr.open(path,"r+b"))
			{
				throw Error(i18n("Failed to write last chunk to DND file : %1").arg(fptr.errorString()));
			}
		}
		
		DNDFileHeader hdr;
		fptr.read(&hdr,sizeof(DNDFileHeader));
		hdr.last_size = lc_size;
		Uint8* tmp = new Uint8[hdr.first_size + hdr.last_size];
		try
		{	
			// put everything in tmp buf
			memcpy(tmp + hdr.first_size,buf,lc_size);
			if (hdr.first_size > 0)
			{
				fptr.seek(File::BEGIN,sizeof(DNDFileHeader));
				fptr.read(tmp,hdr.first_size);
			}
				
			// update the hash of the header
		//	SHA1Hash h = SHA1Hash::generate(tmp,hdr.first_size + hdr.last_size);
		//	memcpy(hdr.data_sha1,h.getData(),20);
				
			// write header + data
			fptr.seek(File::BEGIN,0);
			fptr.write(&hdr,sizeof(DNDFileHeader));
			fptr.write(tmp,hdr.first_size + hdr.last_size);
			delete [] tmp;
		}
		catch (...)
		{
			delete [] tmp;
			throw;
		}
	}

}
