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

#include <qfile.h>
#include "config.h"
#include <klocale.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <torrent/globals.h>
#include "file.h"
#include "error.h"
#include "log.h"

namespace bt
{

	File::File() : fptr(0)
	{}


	File::~File() 
	{
		close();
	}
	
	bool File::open(const QString & file,const QString & mode)
	{
		this->file = file;
		if (fptr)
			close();
#if HAVE_FOPEN64
		fptr = fopen64(QFile::encodeName(file),mode.ascii());
#else
		fptr = fopen(QFile::encodeName(file),mode.ascii());
#endif
		return fptr != 0;
	}
		
	void File::close()
	{
		if (fptr)
		{
			fclose(fptr);
			fptr = 0;
		}
	}
	
	void File::flush()
	{
		if (fptr)
			fflush(fptr);
	}

	Uint32 File::write(const void* buf,Uint32 size)
	{
		if (!fptr)
			return 0;

		Uint32 ret = fwrite(buf,1,size,fptr);
		if (ret != size)
		{
			if (errno == ENOSPC)
				Out() << "Disk full !" << endl;
			
			throw Error(i18n("Cannot write to %1 : %2").arg(file).arg(strerror(errno)));
		}
		return ret;
	}
	
	Uint32 File::read(void* buf,Uint32 size)
	{
		if (!fptr)
			return 0;

		Uint32 ret = fread(buf,1,size,fptr);
		if (ferror(fptr))
		{
			clearerr(fptr);
			throw Error(i18n("Cannot read from %1").arg(file));
		}
		return ret;
	}

	Uint64 File::seek(SeekPos from,Int64 num)
	{
	//	printf("sizeof(off_t) = %i\n",sizeof(__off64_t));
		if (!fptr)
			return 0;
		
		int p = SEEK_CUR; // use a default to prevent compiler warning
		switch (from)
		{
			case BEGIN : p = SEEK_SET; break;
			case END : p = SEEK_END; break;
			case CURRENT : p = SEEK_CUR; break;
			default:
				break;
		}
#if HAVE_FSEEKO64
		fseeko64(fptr,num,p);
		return ftello64(fptr);
#else
		fseeko(fptr,num,p);
		return ftello(fptr);
#endif
	}

	bool File::eof() const
	{
		if (!fptr)
			return true;
		
		return feof(fptr) != 0;
	}

	Uint64 File::tell() const
	{
		if (!fptr)
			return 0;
		
		return ftello(fptr);
	}

	QString File::errorString() const
	{
		return QString(strerror(errno));
	}
}
