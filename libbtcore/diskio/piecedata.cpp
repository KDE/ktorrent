/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <util/log.h>
#include "piecedata.h"
#include "cachefile.h"
#include "chunk.h"

namespace bt
{

	PieceData::PieceData(Chunk* chunk,Uint32 off,Uint32 len,Uint8* ptr,CacheFile* file) 
		: chunk(chunk),off(off),len(len),ptr(ptr),file(file),ref_count(0)
	{
	}

	PieceData::~PieceData()
	{
		ref_count = 0;
		if (ptr)
			unload();
	}

	void PieceData::unload()
	{
		if (ref_count > 0)
			return;
		
		if (!file)
			delete [] ptr;
		else
			file->unmap(ptr,len);
		ptr = 0;
	}

	void PieceData::unmapped()
	{
		ptr = 0;
		Out(SYS_DIO|LOG_DEBUG) << QString("Piece %1 %2 %3 unmapped").arg(chunk->getIndex()).arg(off).arg(len) << endl;
	}
	
	PieceDataPtr::PieceDataPtr(PieceData* pdata) : pdata(pdata)
	{
		if (pdata)
			pdata->ref();
	}

	PieceDataPtr::PieceDataPtr(const bt::PieceDataPtr& other) : pdata(other.pdata)
	{
		if (pdata)
			pdata->ref();
	}

	PieceDataPtr::~PieceDataPtr()
	{
		if (pdata)
			pdata->unref();
	}

	PieceDataPtr& PieceDataPtr::operator=(const bt::PieceDataPtr& other)
	{
		if (pdata)
			pdata->unref();
		
		pdata = other.pdata;
		if (pdata)
			pdata->ref();
		
		return *this;
	}




}
